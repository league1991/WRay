#include "StdAfx.h"
#include "Clipper.h"

WBoxClipper::WBoxClipper(void)
{
}

WBoxClipper::~WBoxClipper(void)
{
}

WBoxClipper::WBoxClipper( const WBoundingBox& box )
{
	clipBox = box;
	boxArray[0][0] = clipBox.pMin.x;
	boxArray[0][1] = clipBox.pMin.y;
	boxArray[0][2] = clipBox.pMin.z;
	boxArray[1][0] = clipBox.pMax.x;
	boxArray[1][1] = clipBox.pMax.y;
	boxArray[1][2] = clipBox.pMax.z;
}

void WBoxClipper::setClipBox( const WBoundingBox& box )
{
	clipBox = box;
	boxArray[0][0] = clipBox.pMin.x;
	boxArray[0][1] = clipBox.pMin.y;
	boxArray[0][2] = clipBox.pMin.z;
	boxArray[1][0] = clipBox.pMax.x;
	boxArray[1][1] = clipBox.pMax.y;
	boxArray[1][2] = clipBox.pMax.z;
}
void WBoxClipper::setClipBox(float box[2][3])
{
	memcpy((void*)boxArray, (void*)box, sizeof(float)* 6);
	memcpy((void*)clipBox.pMin.v, (void*)box, sizeof(float)*3);
	memcpy((void*)clipBox.pMax.v, (void*)box[1], sizeof(float)*3);
}

void WBoxClipper::displayPolygon( vector<WVector3>& vertices )
{
	glBegin(GL_LINE_LOOP);
	for(unsigned int i = 0; i< vertices.size(); i++)
		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
	glEnd();
	glBegin(GL_POINTS);
	for(unsigned int i = 0; i< vertices.size(); i++)
		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
	glEnd();
}


bool WBoxClipper::clipTriangle(const WTriangle& tri, vector<WVector3>& outVertices)
{
	// 确定包围盒和三角形类型
	// 对于包围盒： 0表示x轴向厚度为0， 1表示y轴向厚度为0，2表示z轴向厚度为0, 3表示一般情况
	// 对于三角形： 0表示法向量在x轴向，1表示法向量在y轴向，2表示法向量在z轴向，3表示一般情况
	char boxType = 3, triType = 3;
	for (char i = 0; i < 3; ++i)
	{
		if (boxArray[0][i] == boxArray[1][i])
			boxType = i;
		if (tri.point1.v[i] == tri.point2.v[i] && 
			tri.point2.v[i] == tri.point3.v[i])
			triType = i;
	}

	// 提早退出
	// 注意：如果三角形不是轴对齐的，由于浮点误差仍然可能与包围盒相交
	if (boxType < 3 && triType < 3 && 
		(triType != boxType || (triType == boxType && tri.point1.v[triType] != boxArray[0][boxType])))
		return false;

	char order[3] = {0, 1, 2};
	if (triType == 3)
	{
		// 按照坐标范围大小确定裁剪顺序，减少因为浮点误差引起的错误
		float triBox[2][3];
		triBox [0][0] = triBox[1][0] = tri.point1.x;
		triBox [0][1] = triBox[1][1] = tri.point1.y;
		triBox [0][2] = triBox[1][2] = tri.point1.z;

		triBox [0][0] = minf(triBox [0][0], tri.point2.x);
		triBox [0][1] = minf(triBox [0][1], tri.point2.y);
		triBox [0][2] = minf(triBox [0][2], tri.point2.z);
		triBox [1][0] = maxf(triBox [1][0], tri.point2.x);
		triBox [1][1] = maxf(triBox [1][1], tri.point2.y);
		triBox [1][2] = maxf(triBox [1][2], tri.point2.z);

		triBox [0][0] = minf(triBox [0][0], tri.point3.x);
		triBox [0][1] = minf(triBox [0][1], tri.point3.y);
		triBox [0][2] = minf(triBox [0][2], tri.point3.z);
		triBox [1][0] = maxf(triBox [1][0], tri.point3.x);
		triBox [1][1] = maxf(triBox [1][1], tri.point3.y);
		triBox [1][2] = maxf(triBox [1][2], tri.point3.z);

		triBox [0][0] = abs(triBox[0][0] - triBox[1][0]);
		triBox [0][1] = abs(triBox[0][1] - triBox[1][1]);
		triBox [0][2] = abs(triBox[0][2] - triBox[1][2]);
		for (int i = 0; i < 3; ++i)
			for (int j = i + 1; j < 3; ++j)
				if (triBox[0][i] < triBox[0][j])
					swap(order[i], order[j]);
	}
	// 准备数据
	// 由于三角形被AABB裁剪得到的多边形边数最多为9，故准备一个大小为9的二维数组
	float polygon0[9][3], polygon1[9][3];

	polygon0[0][0] = tri.point1.x;
	polygon0[0][1] = tri.point1.y;
	polygon0[0][2] = tri.point1.z;

	polygon0[1][0] = tri.point2.x;
	polygon0[1][1] = tri.point2.y;
	polygon0[1][2] = tri.point2.z;

	polygon0[2][0] = tri.point3.x;
	polygon0[2][1] = tri.point3.y;
	polygon0[2][2] = tri.point3.z;

	float (*srcPoly)[3] = polygon0, (*dstPoly)[3] = polygon1;
	char numEdges = 3;
	char otherAxisTable[3][2] = {{1,2},{0,2},{0,1}};

	// 开始裁剪，使用AABB的6个面裁剪
	for (char i = 0; i < 6; i++)
	{	// 循环顺序为 minX-maxX-minY-maxY-minZ-maxZ
		// 选定裁剪轴向以及裁剪位置
		char axis = order[i >> 1];		// (0 1 2) --> (x y z)   
		char boxPoint = i & 0x1;		// (0 1)   --> (pMin pMax)
		// 若当前轴向为包围盒厚度为0的轴向，不进行裁剪
		if (axis == boxType)
			continue;
		// 在当前选定的轴向和位置，对多边形进行裁剪
		// newEdges记录裁剪后的多边形边数
		char newNumEdges = 0;
		for (int curPoint = 0; curPoint < numEdges; curPoint++)
		{
			char nextPoint = (curPoint + 1) % numEdges;
			char isCurIn   = ((srcPoly[curPoint][axis] > boxArray[boxPoint][axis]) ^ boxPoint) |
				(srcPoly[curPoint][axis] == boxArray[boxPoint][axis]);
			char isNextIn  = ((srcPoly[nextPoint][axis] > boxArray[boxPoint][axis]) ^ boxPoint) |
				(srcPoly[nextPoint][axis] == boxArray[boxPoint][axis]);
			if (isCurIn & isNextIn)
			{	// 当前点和下一个点都在剪切平面内，应该把下一个点加入目标多边形
				dstPoly[newNumEdges][0] = srcPoly[nextPoint][0];
				dstPoly[newNumEdges][1] = srcPoly[nextPoint][1];
				dstPoly[newNumEdges][2] = srcPoly[nextPoint][2];
				++newNumEdges;
			}
			else if (isCurIn & ~isNextIn & (srcPoly[curPoint][axis] != boxArray[boxPoint][axis]))
			{	// 一条边向外穿越了剪切平面，而且当前点不在平面上，应该把交点加入目标多边形
				float t = (boxArray[boxPoint][axis] - srcPoly[curPoint][axis]) / 
					(srcPoly[nextPoint][axis] - srcPoly[curPoint][axis]);
				// 当前轴向直接赋值，这样防止浮点误差使得实际交点不在剪切平面上的情况
				dstPoly[newNumEdges][axis] = boxArray[boxPoint][axis];
				// 计算交点在其他两个轴向的坐标并加入目标多边形
				char otherAxis = otherAxisTable[axis][0];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				otherAxis = otherAxisTable[axis][1];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				++newNumEdges;
			}
			else if (~isCurIn & isNextIn)
			{	// 一条边向内穿越了剪切平面，应该把交点以及下一个点加入目标多边形
				float t = (boxArray[boxPoint][axis] - srcPoly[curPoint][axis]) / 
					(srcPoly[nextPoint][axis] - srcPoly[curPoint][axis]);
				// 当前轴向直接赋值，这样防止浮点误差使得实际交点不在剪切平面上的情况
				dstPoly[newNumEdges][axis] = boxArray[boxPoint][axis];
				// 计算交点在其他两个轴向的坐标并加入目标多边形
				char otherAxis = otherAxisTable[axis][0];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				otherAxis = otherAxisTable[axis][1];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				++newNumEdges;
				// 如果下一个点不在剪切平面上，把下一个点也加入目标多边形
				if (srcPoly[nextPoint][axis] != boxArray[boxPoint][axis])
				{
					dstPoly[newNumEdges][0] = srcPoly[nextPoint][0];
					dstPoly[newNumEdges][1] = srcPoly[nextPoint][1];
					dstPoly[newNumEdges][2] = srcPoly[nextPoint][2];
					++newNumEdges;
				}
			}
		}
		numEdges = newNumEdges;
		float (*temp)[3] = srcPoly;
		srcPoly = dstPoly;
		dstPoly = temp;
	}
	// 边数为0说明三角形跟AABB不相交
	if (!numEdges)
		return false;
	// 新建结果的包围盒
	for (char e = 0; e < numEdges; e++)
	{
		outVertices.push_back(WVector3(srcPoly[e][0],srcPoly[e][1],srcPoly[e][2]));
	}
	return true;
}

void WBoxClipper::drawPolygon( vector<WVector3>& vertices )
{
	glBegin(GL_POINTS);
	for (unsigned i = 0; i < vertices.size(); i++)
	{
		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
	}
	glEnd();
	glBegin(GL_LINE_LOOP);
	for (unsigned i = 0; i < vertices.size(); i++)
	{
		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
//		cout << "vertices " << i << endl;
//		vertices[i].showCoords();
	}
	glEnd();
}

bool WBoxClipper::getClipBox(const WTriangle& tri, float resultBox[2][3])
{
	// 确定包围盒和三角形类型
	// 对于包围盒： 0表示x轴向厚度为0， 1表示y轴向厚度为0，2表示z轴向厚度为0, 3表示一般情况
	// 对于三角形： 0表示法向量在x轴向，1表示法向量在y轴向，2表示法向量在z轴向，3表示一般情况
	char boxType = 3, triType = 3;
	for (char i = 0; i < 3; ++i)
	{
		if (boxArray[0][i] == boxArray[1][i])
			boxType = i;
		if (tri.point1.v[i] == tri.point2.v[i] && 
			tri.point2.v[i] == tri.point3.v[i])
			triType = i;
	}

	// 提早退出
	// 注意：如果三角形不是轴对齐的，由于浮点误差仍然可能与包围盒相交
	if (boxType < 3 && triType < 3 && 
		(triType != boxType || (triType == boxType && tri.point1.v[triType] != boxArray[0][boxType])))
		return false;

	char order[3] = {0, 1, 2};
	if (triType == 3)
	{
		// 按照坐标范围大小确定裁剪顺序，减少因为浮点误差引起的错误
		float triBox[2][3];
		triBox [0][0] = triBox[1][0] = tri.point1.x;
		triBox [0][1] = triBox[1][1] = tri.point1.y;
		triBox [0][2] = triBox[1][2] = tri.point1.z;

		triBox [0][0] = minf(triBox [0][0], tri.point2.x);
		triBox [0][1] = minf(triBox [0][1], tri.point2.y);
		triBox [0][2] = minf(triBox [0][2], tri.point2.z);
		triBox [1][0] = maxf(triBox [1][0], tri.point2.x);
		triBox [1][1] = maxf(triBox [1][1], tri.point2.y);
		triBox [1][2] = maxf(triBox [1][2], tri.point2.z);

		triBox [0][0] = minf(triBox [0][0], tri.point3.x);
		triBox [0][1] = minf(triBox [0][1], tri.point3.y);
		triBox [0][2] = minf(triBox [0][2], tri.point3.z);
		triBox [1][0] = maxf(triBox [1][0], tri.point3.x);
		triBox [1][1] = maxf(triBox [1][1], tri.point3.y);
		triBox [1][2] = maxf(triBox [1][2], tri.point3.z);

		triBox [0][0] = abs(triBox[0][0] - triBox[1][0]);
		triBox [0][1] = abs(triBox[0][1] - triBox[1][1]);
		triBox [0][2] = abs(triBox[0][2] - triBox[1][2]);
		for (int i = 0; i < 3; ++i)
			for (int j = i + 1; j < 3; ++j)
				if (triBox[0][i] < triBox[0][j])
					swap(order[i], order[j]);
	}
	// 准备数据
	// 由于三角形被AABB裁剪得到的多边形边数最多为9，故准备一个大小为9的二维数组
	float polygon0[9][3], polygon1[9][3];

	polygon0[0][0] = tri.point1.x;
	polygon0[0][1] = tri.point1.y;
	polygon0[0][2] = tri.point1.z;

	polygon0[1][0] = tri.point2.x;
	polygon0[1][1] = tri.point2.y;
	polygon0[1][2] = tri.point2.z;

	polygon0[2][0] = tri.point3.x;
	polygon0[2][1] = tri.point3.y;
	polygon0[2][2] = tri.point3.z;

	float (*srcPoly)[3] = polygon0, (*dstPoly)[3] = polygon1;
	char numEdges = 3;
	char otherAxisTable[3][2] = {{1,2},{0,2},{0,1}};

	// 开始裁剪，使用AABB的6个面裁剪
	for (char i = 0; i < 6; i++)
	{	// 循环顺序为 minX-maxX-minY-maxY-minZ-maxZ
		// 选定裁剪轴向以及裁剪位置
		char axis = order[i >> 1];		// (0 1 2) --> (x y z)   
		char boxPoint = i & 0x1;		// (0 1)   --> (pMin pMax)
		// 若当前轴向为包围盒厚度为0的轴向，不进行裁剪
		if (axis == boxType)
			continue;
		// 在当前选定的轴向和位置，对多边形进行裁剪
		// newEdges记录裁剪后的多边形边数
		char newNumEdges = 0;
		for (int curPoint = 0; curPoint < numEdges; curPoint++)
		{
			char nextPoint = (curPoint + 1) % numEdges;
			char isCurIn   = ((srcPoly[curPoint][axis] > boxArray[boxPoint][axis]) ^ boxPoint) |
				             (srcPoly[curPoint][axis] == boxArray[boxPoint][axis]);
			char isNextIn  = ((srcPoly[nextPoint][axis] > boxArray[boxPoint][axis]) ^ boxPoint) |
				             (srcPoly[nextPoint][axis] == boxArray[boxPoint][axis]);
			if (isCurIn & isNextIn)
			{	// 当前点和下一个点都在剪切平面内，应该把下一个点加入目标多边形
				dstPoly[newNumEdges][0] = srcPoly[nextPoint][0];
				dstPoly[newNumEdges][1] = srcPoly[nextPoint][1];
				dstPoly[newNumEdges][2] = srcPoly[nextPoint][2];
				++newNumEdges;
			}
			else if (isCurIn & ~isNextIn & (srcPoly[curPoint][axis] != boxArray[boxPoint][axis]))
			{	// 一条边向外穿越了剪切平面，而且当前点不在平面上，应该把交点加入目标多边形
				float t = (boxArray[boxPoint][axis] - srcPoly[curPoint][axis]) / 
					(srcPoly[nextPoint][axis] - srcPoly[curPoint][axis]);
				// 当前轴向直接赋值，这样防止浮点误差使得实际交点不在剪切平面上的情况
				dstPoly[newNumEdges][axis] = boxArray[boxPoint][axis];
				// 计算交点在其他两个轴向的坐标并加入目标多边形
				char otherAxis = otherAxisTable[axis][0];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				otherAxis = otherAxisTable[axis][1];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				++newNumEdges;
			}
			else if (~isCurIn & isNextIn)
			{	// 一条边向内穿越了剪切平面，应该把交点以及下一个点加入目标多边形
				float t = (boxArray[boxPoint][axis] - srcPoly[curPoint][axis]) / 
					(srcPoly[nextPoint][axis] - srcPoly[curPoint][axis]);
				// 当前轴向直接赋值，这样防止浮点误差使得实际交点不在剪切平面上的情况
				dstPoly[newNumEdges][axis] = boxArray[boxPoint][axis];
				// 计算交点在其他两个轴向的坐标并加入目标多边形
				char otherAxis = otherAxisTable[axis][0];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				otherAxis = otherAxisTable[axis][1];
				dstPoly[newNumEdges][otherAxis] = 
					srcPoly[curPoint][otherAxis] + 
					t * (srcPoly[nextPoint][otherAxis] - srcPoly[curPoint][otherAxis]);
				++newNumEdges;
				// 如果下一个点不在剪切平面上，把下一个点也加入目标多边形
				if (srcPoly[nextPoint][axis] != boxArray[boxPoint][axis])
				{
					dstPoly[newNumEdges][0] = srcPoly[nextPoint][0];
					dstPoly[newNumEdges][1] = srcPoly[nextPoint][1];
					dstPoly[newNumEdges][2] = srcPoly[nextPoint][2];
					++newNumEdges;
				}
			}
		}
		numEdges = newNumEdges;
		float (*temp)[3] = srcPoly;
		srcPoly = dstPoly;
		dstPoly = temp;
	}
	// 边数为0说明三角形跟AABB不相交
	if (!numEdges)
		return false;

	// 新建结果的包围盒
	resultBox[0][0] = resultBox[1][0] = srcPoly[0][0];
	resultBox[0][1] = resultBox[1][1] = srcPoly[0][1];
	resultBox[0][2] = resultBox[1][2] = srcPoly[0][2];
	for (char e = 1; e < numEdges; e++)
	{
		resultBox[0][0] = minf(resultBox[0][0], srcPoly[e][0]);
		resultBox[0][1] = minf(resultBox[0][1], srcPoly[e][1]);
		resultBox[0][2] = minf(resultBox[0][2], srcPoly[e][2]);
		resultBox[1][0] = maxf(resultBox[1][0], srcPoly[e][0]);
		resultBox[1][1] = maxf(resultBox[1][1], srcPoly[e][1]);
		resultBox[1][2] = maxf(resultBox[1][2], srcPoly[e][2]);
	}
	// 防止浮点误差导致结果包围盒超出裁剪包围盒
	resultBox[0][0] = maxf(resultBox[0][0], boxArray[0][0]);
	resultBox[0][1] = maxf(resultBox[0][1], boxArray[0][1]);
	resultBox[0][2] = maxf(resultBox[0][2], boxArray[0][2]);
	resultBox[1][0] = minf(resultBox[1][0], boxArray[1][0]);
	resultBox[1][1] = minf(resultBox[1][1], boxArray[1][1]);
	resultBox[1][2] = minf(resultBox[1][2], boxArray[1][2]);
	return true;
}
