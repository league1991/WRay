#include "StdAfx.h"
#include "SimpleKD.h"

WSimpleKD::WSimpleKD(void)
{
	KT = 1;
	KI = 80;
	triangles = NULL;
	kdInteriors = NULL;
	kdLeafs = NULL;
	leafTriangles = NULL;
	numInteriors = numLeafs = treeDepth = totalTriangles = numLeafTriangles = 0;
	minBoxFactor = SKD_MIN_BOX_FACTOR;
}

WSimpleKD::~WSimpleKD(void)
{
	clearTree();
}

void WSimpleKD::buildTree( WScene&scene )
{
	//获得场景三角形数组和包围盒
	cout<<"begin to build KD"<<endl;
	scene.getTriangleArray(triangles, totalTriangles);
	cout<<"total triangles: "<<totalTriangles<<endl;
	sceneBox = scene.getBBox();

	// 添加event
	vector<WBoundingEdge> edgeList;
	edgeList.reserve(totalTriangles * 3 * 2);
	for (int ithTri = 0; ithTri < totalTriangles; ++ithTri)
	{
		// 计算三角形的包围盒
		float triBox[2][3];
		triBox [0][0] = triBox[1][0] = triangles[ithTri].point1.x;
		triBox [0][1] = triBox[1][1] = triangles[ithTri].point1.y;
		triBox [0][2] = triBox[1][2] = triangles[ithTri].point1.z;

		triBox [0][0] = minf(triBox [0][0], triangles[ithTri].point2.x);
		triBox [0][1] = minf(triBox [0][1], triangles[ithTri].point2.y);
		triBox [0][2] = minf(triBox [0][2], triangles[ithTri].point2.z);
		triBox [1][0] = maxf(triBox [1][0], triangles[ithTri].point2.x);
		triBox [1][1] = maxf(triBox [1][1], triangles[ithTri].point2.y);
		triBox [1][2] = maxf(triBox [1][2], triangles[ithTri].point2.z);

		triBox [0][0] = minf(triBox [0][0], triangles[ithTri].point3.x);
		triBox [0][1] = minf(triBox [0][1], triangles[ithTri].point3.y);
		triBox [0][2] = minf(triBox [0][2], triangles[ithTri].point3.z);
		triBox [1][0] = maxf(triBox [1][0], triangles[ithTri].point3.x);
		triBox [1][1] = maxf(triBox [1][1], triangles[ithTri].point3.y);
		triBox [1][2] = maxf(triBox [1][2], triangles[ithTri].point3.z);

		// 根据包围盒生成BoundingEdge,并插入edgeList
		for (int axis = 0; axis <3; ++axis)
		{
			if (triBox [0][axis] == triBox[1][axis])
				edgeList.push_back(WBoundingEdge(WBoundingEdge::BE_PLANAR,WBoundingEdge::EdgeAxis(axis),triBox[0][axis], ithTri));
			else
			{
				edgeList.push_back(WBoundingEdge(WBoundingEdge::BE_START,WBoundingEdge::EdgeAxis(axis),triBox[0][axis], ithTri));
				edgeList.push_back(WBoundingEdge(WBoundingEdge::BE_END,WBoundingEdge::EdgeAxis(axis),triBox[1][axis], ithTri));
			}
		}
	}
	// 对boundingEdge排序
	sort(edgeList.begin(), edgeList.end());
	//edgeList.sort();

	// 新建用于存放节点的内存
	int approNodes = max(totalTriangles, 5000) * 10;
	kdInteriors = new WSKDInterior[approNodes];	// 内部节点数组
	kdLeafs     = new WSKDLeaf[approNodes];		// 叶节点数组
	leafTriangles = new WTriangle*[approNodes];	// 叶节点三角形指针数组

	// 递归构建KD树
	float nodeBox[2][3];
	for (int i = 0; i < 3; i++)
	{
		nodeBox[0][i] = sceneBox.pMin.v[i];
		nodeBox[1][i] = sceneBox.pMax.v[i];
	}
	minBoxSize[0] = (nodeBox[1][0] - nodeBox[0][0]) * minBoxFactor;
	minBoxSize[1] = (nodeBox[1][1] - nodeBox[0][1]) * minBoxFactor;
	minBoxSize[2] = (nodeBox[1][2] - nodeBox[0][2]) * minBoxFactor;
	char* triangleMap = new char[totalTriangles];
	buildTreeKernel(edgeList, totalTriangles, 0, nodeBox, triangleMap);
	delete[] triangleMap;
}
WSKDNode* WSimpleKD::buildTreeKernel( vector<WBoundingEdge>&edges, 
								int nTris,
								int depth, 
								float nodeBox[2][3],
								char triangleMap[])
{
	//printf("depth: %d\n", depth);
	// 更新树的最大深度以及包围盒数组
	treeDepth = depth > treeDepth ? depth : treeDepth;
	//nodeBoxes.push_back(WBoundingBox(nodeBox));

	// 计算包围盒大小
	float boxSize[3];
	boxSize[0] = (nodeBox[1][0] - nodeBox[0][0]);
	boxSize[1] = (nodeBox[1][1] - nodeBox[0][1]);
	boxSize[2] = (nodeBox[1][2] - nodeBox[0][2]);
	// 准备记录最佳位置的数据
	float bestSAH = FLT_MAX;		// 最好开销
	float bestT   = 0;
	char  bestSide = 0;				// 分隔平面上的节点归属
	char  bestAxis = 0;
	int   bestLTri = nTris, bestRTri = 0;
	vector<WBoundingEdge>::iterator bestP = edges.end();// 最佳分隔平面

	//printf("begin to find best plane\n");
	// 找出最佳的分隔平面，在3个维度分别搜索
	for (int axis = 0; axis < 3; ++axis)
	{
		int NL = 0, NP = 0, NR = nTris;
		for (vector<WBoundingEdge>::iterator pE = edges.begin();
			 pE != edges.end();)
		{
			// 跳过不同轴向的BoundingEdge
			if (pE->axis != axis)
			{
				++pE;
				continue;
			}

			// 到达当前轴向的一个BoundingEdge
			// 首先统计在当前参数位置的三种BE数量
			vector<WBoundingEdge>::iterator p = pE;
			int nStart = 0, nEnd = 0, nPlanar = 0;
			while (pE != edges.end() && pE->t == p->t)
			{
				// 只统计当前轴向的BE
				if (pE->axis == axis)
				{
					switch (pE->type )
					{
					case WBoundingEdge::BE_START:
						nStart++;	break;
					case WBoundingEdge::BE_PLANAR:
						nPlanar++;	break;
					case WBoundingEdge::BE_END:
						nEnd++;		break;
					}
				}
				++pE;
			}

			NP = nPlanar;			// 当前位置的平面边独立出来单独计算
			NR -= nPlanar;
			NR -= nEnd;				// 当前位置的结束边归到左节点

			// 现在得到当前分割平面左右的三角形个数
			// 要计算最佳的分隔
			float PL, PR;				// 命中包围盒左右两边的概率
			if (boxSize[axis] == 0.0f)	// 包围盒厚度为0时，认为左右两边概率相等
				PL = PR = 1.0f;
			else
			{							// 否则概率为左右两边的表面积比
				PL = (p->t - nodeBox[0][axis]) / boxSize[axis];
				PR = (nodeBox[1][axis] - p->t) / boxSize[axis];
			}

			// 分别考虑平面边归到左右两边子节点的情况，选出最优的一种
			float SAH0 = KT + KI * (PL * (NL + NP) + PR * NR);
			float SAH1 = KT + KI * (PL * NL + PR * (NP + NR));
			if ((NL + NP == 0 && PL != 0) || (NR == 0 && PR != 0))
				SAH0 *= 0.8f;
			if ((NR + NP == 0 && PR != 0) || (NL == 0 && PL != 0))
				SAH1 *= 0.8f;
			char side = SAH0 < SAH1 ? 0 : 1;
			float SAH = minf(SAH0, SAH1);

			// 发现更优的分隔方案
			if (SAH < bestSAH)
			{
				bestP = p;  bestSAH = SAH;  bestSide = side;
				bestT = p->t; bestAxis = axis;
				bestLTri = SAH0 < SAH1 ? (NL + NP) : NL;
				bestRTri = SAH0 < SAH1 ? NR : (NR + NP);
			}
			NL += (nStart + nPlanar);
		}
	}
	if (bestT < sceneBox.pMin.v[bestAxis] || bestT > sceneBox.pMax.v[bestAxis] )
	{
		printf("error!");
	}
	// 根据最佳的分隔平面,确定每个三角形的归属
	//map<int, char>triangleMap;  // 三角形序号 --> 类型（0左边，1右边，2两边）
	// 把所有三角形都标记为两边
	for (vector<WBoundingEdge>::iterator pE = edges.begin();pE != edges.end(); ++pE)
		triangleMap[pE->triangleID] = 2;
	/*
	if (depth > 50)
	{
		printf("depth %d\n", depth);
	}*/
	// 下列情况新建叶节点
	// 1. 找不到任何分隔
	// 2. 最佳的分隔方案比不分割开销要大
	// 3. 树的深度超出最大值
	// 4. 三角形数量少于推荐值
	// 5. 最佳分隔方案使得有一边为体积为0的空节点
	// 6. 最佳分隔方案使得一边节点体积和三角形数都不减少
	// 7. 节点大小小于一定值
	if (bestSAH == FLT_MAX || bestSAH > KI * nTris || 
		depth > SKD_MAX_DEPTH || nTris <= SKD_RECOMM_LEAF_TRIS ||
		(bestP->t == nodeBox[0][bestAxis] && bestLTri == 0) ||
		(nodeBox[1][bestAxis] == bestP->t && bestRTri == 0) ||
		(bestP->t - nodeBox[0][bestAxis] == boxSize[bestAxis] && bestLTri == nTris) ||
		(nodeBox[1][bestAxis] - bestP->t == boxSize[bestAxis] && bestRTri == nTris) ||
		(boxSize[0] < minBoxSize[0] && boxSize[1] < minBoxSize[1] && boxSize[2] < minBoxSize[2]))
	{/*
		if (bestSAH == FLT_MAX || depth > SKD_MAX_DEPTH)
			printf("warning: tree split problem.\n");*/
		WSKDLeaf * leaf = &kdLeafs[numLeafs];
		leaf->type = WSKDNode::KDN_LEAF;
		leaf->triangle = nTris != 0 ? &leafTriangles[numLeafTriangles] : NULL;
		memcpy((void*)leaf->box, (void*)nodeBox, sizeof(float) * 6);
		set<int> triangleSet;
		for (vector<WBoundingEdge>::iterator pE = edges.begin(); pE != edges.end(); ++pE)
			triangleSet.insert(pE->triangleID);
		for (set<int>::iterator pS = triangleSet.begin(); pS != triangleSet.end(); ++pS)
			leafTriangles[numLeafTriangles++] = triangles + *pS;
		leaf->nTriangles = triangleSet.size();
		++numLeafs;
		return leaf;
	}

	//printf("begin to mark triangle\n");
	for (vector<WBoundingEdge>::iterator pE = edges.begin();
		pE != edges.end(); ++pE)
	{
		if (pE->axis == WBoundingEdge::EdgeAxis(bestAxis))
		{
			if (pE->type == WBoundingEdge::BE_END && pE->t <= bestT)
				triangleMap[pE->triangleID]  = 0;
			else if (pE->type == WBoundingEdge::BE_START && pE->t >= bestT)
				triangleMap[pE->triangleID]  = 1;
			else if (pE->type == WBoundingEdge::BE_PLANAR)
			{
				if (pE->t < bestT || (pE->t == bestT && bestSide == 0))
					triangleMap[pE->triangleID]  = 0;
				else if (pE->t > bestT || (pE->t == bestT && bestSide == 1))
					triangleMap[pE->triangleID]  = 1;
			}
		}
	}

	//printf("begin to generate edge\n");
	// 再扫描一次event，把只属于一边节点的三角形对应的event加入对应子节点event列表
	// 并记录横跨两个节点的三角形
	vector<WBoundingEdge> Ledges, Redges;
	Ledges.reserve(edges.size());
	Redges.reserve(edges.size());
	set<int> midTris;
	for (vector<WBoundingEdge>::iterator pE = edges.begin(); pE != edges.end();)
	{
		if (triangleMap[pE->triangleID] == 0)
			//Ledges.splice(Ledges.end(), edges, pE++);
			Ledges.push_back(*(pE++));
		else if (triangleMap[pE->triangleID] == 1)
			//Redges.splice(Redges.end(), edges, pE++);
			Redges.push_back(*(pE++));
		else
		{
			midTris.insert(pE->triangleID);
			++pE;
		}
	}

	// 对于横跨两个节点的三角形，计算新的BE
	// 得到左右节点的包围盒
	float LNodeBox[2][3];float RNodeBox[2][3];
	memcpy((void*)LNodeBox, (void*)nodeBox, sizeof(float) * 6);
	memcpy((void*)RNodeBox, (void*)nodeBox, sizeof(float) * 6);
	LNodeBox[1][bestAxis] = RNodeBox[0][bestAxis] = bestT;

	//printf("begin to clip triangle\n");
	// 使用包围盒进行裁剪并把结果加入暂存的BE列表
	WBoxClipper clipper0, clipper1;
	clipper0.setClipBox(LNodeBox);
	clipper1.setClipBox(RNodeBox);
	vector<WBoundingEdge> tempEdgesL, tempEdgesR;
	tempEdgesL.reserve(midTris.size() * 2 * 3);
	tempEdgesR.reserve(midTris.size() * 2 * 3);
	for (set<int>::iterator pT = midTris.begin();
		 pT != midTris.end(); ++pT)
	{
		WTriangle* pTri = triangles + *pT;
		float LRes[2][3], RRes[2][3];
		bool isLIn = clipper0.getClipBox(*pTri, LRes);
		bool isRIn = clipper1.getClipBox(*pTri, RRes);
		for(char i = 0; i < 3; i++)
		{
			// 理论上所有midTris中的三角形都与左右节点相交
			// 但是实际上由于浮点误差，可能出现本来不相交的三角形被加入节点
			// 因此要通过进一步判断，防止出现多余的三角形
			WBoundingEdge::EdgeAxis a = WBoundingEdge::EdgeAxis(i);
			if (isLIn)			// 判断是否与左节点相交，防止浮点误差引起实际上不相交的三角形加入节点
				if (LRes[0][i] == LRes[1][i])
					tempEdgesL.push_back(WBoundingEdge(WBoundingEdge::BE_PLANAR, a, LRes[0][i], *pT));
				else
				{
					tempEdgesL.push_back(WBoundingEdge(WBoundingEdge::BE_START, a, LRes[0][i], *pT));
					tempEdgesL.push_back(WBoundingEdge(WBoundingEdge::BE_END, a, LRes[1][i], *pT));
				}
			if (isRIn)
				if (RRes[0][i] == RRes[1][i])
					tempEdgesR.push_back(WBoundingEdge(WBoundingEdge::BE_PLANAR, a, RRes[0][i], *pT));
				else
				{
					tempEdgesR.push_back(WBoundingEdge(WBoundingEdge::BE_START, a, RRes[0][i], *pT));
					tempEdgesR.push_back(WBoundingEdge(WBoundingEdge::BE_END, a, RRes[1][i], *pT));
				}
		}
		if (!isLIn || !isRIn)
		{
			printf("error!!!");
		}
	}
	
	//printf("begin to sort and merge edge\n");
	// 把新的BE加入原来的集合中，组成子节点新的BE集合
	sort(tempEdgesL.begin(), tempEdgesL.end());
	sort(tempEdgesR.begin(), tempEdgesR.end());
	/*
	tempEdgesL.sort();
	tempEdgesR.sort();
	Ledges.merge(tempEdgesL);
	Redges.merge(tempEdgesR);*/
	vector<WBoundingEdge> L, R;
	unsigned s = Ledges.size() + tempEdgesL.size();
	L.resize(s);
	s = Redges.size() + tempEdgesR.size();
	R.resize(s);
	merge(Ledges.begin(), Ledges.end(), tempEdgesL.begin(), tempEdgesL.end(), L.begin());
	merge(Redges.begin(), Redges.end(), tempEdgesR.begin(), tempEdgesR.end(), R.begin());

	// 存储当前节点
	WSKDInterior* pNode = kdInteriors + numInteriors;
	numInteriors++;
	pNode->type = WSKDNode::NodeType(bestAxis);
	pNode->splitPlane = bestT;
	pNode->leftChild = buildTreeKernel(L, bestLTri, depth + 1, LNodeBox, triangleMap);
	pNode->rightChild = buildTreeKernel(R, bestRTri, depth + 1, RNodeBox, triangleMap);
	return pNode;
}

void WSimpleKD::clearTree()
{
	triangles = NULL;
	delete []kdInteriors;
	delete []kdLeafs;
	delete []leafTriangles;
	kdInteriors = NULL;
	kdLeafs = NULL;
	leafTriangles = NULL;
	numInteriors = numLeafs = treeDepth = totalTriangles = numLeafTriangles = 0;
}

void WSimpleKD::drawTree( unsigned int nthBox/*=0*/, float R /*= 0.7*/, float G /*= 0.7*/, float B /*= 0.7*/ )
{
//	cout<<"\nnode size:"<<nodes.size()<<endl;
	glColor3f(R, G, B);
	drawTreeRecursive(kdInteriors, sceneBox);
}

void WSimpleKD::drawTreeRecursive( WSKDNode* node, const WBoundingBox& box )
{
	if (!numInteriors)
		return;
	box.draw();
	WBoundingBox leftBox, rightBox;
	leftBox = rightBox = box;
	switch(node->type)
	{
	case WSKDNode::KDN_LEAF:
		return;
	case WSKDNode::KDN_XSPLIT:
		leftBox.pMax.x = rightBox.pMin.x = ((WSKDInterior*)node)->splitPlane;
		break;
	case WSKDNode::KDN_YSPLIT:
		leftBox.pMax.y = rightBox.pMin.y = ((WSKDInterior*)node)->splitPlane;
		break;
	case WSKDNode::KDN_ZSPLIT:
		leftBox.pMax.z = rightBox.pMin.z = ((WSKDInterior*)node)->splitPlane;
		break;
	default:
		return;
	}
	drawTreeRecursive(((WSKDInterior*)node)->leftChild, leftBox);
	drawTreeRecursive(((WSKDInterior*)node)->rightChild, rightBox);
}


void WSimpleKD::buildBasicRopes(WSKDNode* pNode, 
							   WSKDNode* ropeX_P, WSKDNode* ropeX_N,
							   WSKDNode* ropeY_P, WSKDNode* ropeY_N,
							   WSKDNode* ropeZ_P, WSKDNode* ropeZ_N)
{
	if (pNode->type == WSKDNode::KDN_LEAF)
	{
		WSKDLeaf* pLeaf = (WSKDLeaf*)pNode;
		pLeaf->ropes[0] = ropeX_P;
		pLeaf->ropes[1] = ropeX_N;
		pLeaf->ropes[2] = ropeY_P;
		pLeaf->ropes[3] = ropeY_N;
		pLeaf->ropes[4] = ropeZ_P;
		pLeaf->ropes[5] = ropeZ_N;
	} 
	else if(pNode->type == WSKDLeaf::KDN_XSPLIT)
	{
		WSKDInterior* pInterior = (WSKDInterior*)pNode;
		buildBasicRopes(pInterior->leftChild,
			pInterior->rightChild, ropeX_N, ropeY_P, ropeY_N, ropeZ_P, ropeZ_N);
		buildBasicRopes(pInterior->rightChild,
			ropeX_P, pInterior->leftChild, ropeY_P, ropeY_N, ropeZ_P, ropeZ_N);
	}
	else if(pNode->type == WSKDLeaf::KDN_YSPLIT)
	{
		WSKDInterior* pInterior = (WSKDInterior*)pNode;
		buildBasicRopes(pInterior->leftChild,
			ropeX_P, ropeX_N, pInterior->rightChild, ropeY_N, ropeZ_P, ropeZ_N);
		buildBasicRopes(pInterior->rightChild,
			ropeX_P, ropeX_N, ropeY_P, pInterior->leftChild, ropeZ_P, ropeZ_N);
	}
	else if(pNode->type == WSKDLeaf::KDN_ZSPLIT)
	{
		WSKDInterior* pInterior = (WSKDInterior*)pNode;
		buildBasicRopes(pInterior->leftChild,
			ropeX_P, ropeX_N, ropeY_P, ropeY_N, pInterior->rightChild, ropeZ_N);
		buildBasicRopes(pInterior->rightChild,
			ropeX_P, ropeX_N, ropeY_P, ropeY_N, ropeZ_P, pInterior->leftChild);
	}
}


bool WSimpleKD::WBoundingEdge::operator<( const WBoundingEdge&e ) const
{
	//不同位置, t值较小的较前
	//两个 t 相等时，确保 planar类型的最前，start类型的较前， end 类型的较后
	//这样做确保在标记三角形的时候先遇到
	//一个三角形的start边，再遇到end边
	return (t < e.t) || ((t == e.t) && (type < e.type));
}

bool WSimpleKD::isIntersect( WRay& r, int beginNode)
{
	return false;
	//	printf("************************\n");
	WVector3 entryPoint;
//	beginNode = 0;
	/*
	for (int i = 0; i < 3; i++)
	{
		//去除-0.0f的可能
		r.direction.v[i] = r.direction.v[i] == 0.0f ? 0.0f : r.direction.v[i];
	}
	if (sceneBox.isInBoxInclusive(r.point))
		entryPoint = r.point;
	else
	{
		float tMin, tMax;
		if (sceneBox.isIntersect(r, tMin, tMax))
		{
			//找到与场景包围盒的交点
			entryPoint = r.point + tMin* r.direction;
		} 
		//与场景包围盒不相交，直接返回
		else
			return false;
	}
	int currNodeID = beginNode, bestTriID = -1;
	while (currNodeID != -1)
	{
//		cout<<"currNodeID:" <<currNodeID<<endl;
		//内部节点
/*		glBegin(GL_POINTS);
		glColor3f(0,0,0);
		glVertex3f(entryPoint.x, entryPoint.y, entryPoint.z);
		glEnd();
		if (nodes[currNodeID]->type != WSKDNode::KDN_LEAF)
		{
			WSKDInterior interior = *((WSKDInterior*)nodes[currNodeID]);
			currNodeID = 
				entryPoint.v[interior.type] < interior.splitPlane ? 
				interior.leftChild : interior.rightChild;
		} 
		//叶节点
		else
		{
			float farT;
			WSKDLeaf& leaf = *((WSKDLeaf*)nodes[currNodeID]);
			int nextNodeID = computeExitFace(leaf, r, entryPoint, farT);
			//找出节点内最近的三角形
			for (int ithTri = 0; ithTri < leaf.nTriangles; ithTri++)
			{
//				cout<<"leaf triangleID "<<leaf.triangleIDs[ithTri]<<endl;
//				triangles[pNode->triangleIDs[ithTri]].showVertexCoords();
				float currT = triangles[leaf.triangleIDs[ithTri]].intersectTest(r);/*
				glColor3f(rand()/32767.0f, 1.0f,1.0f);
				triangles[leaf.triangleIDs[ithTri]].draw(false,true);
				if (currT <= r.tMax && currT > r.tMin)
				{
					//					triangles[leaf.triangleIDs[ithTri]].draw(false,true);
					return true;
					//					cout<<"update triangleID:"<<bestTriID<<endl;
				}
			}
			//如果没有交点，通过rope进入下一个节点
			currNodeID = nextNodeID;
		}
	}
	return false;
*/
}

bool WSimpleKD::intersect( WRay& r,WDifferentialGeometry& DG, 
						 int* endNode, int beginNode)
{
	return false;
	//	printf("************************\n");
//	beginNode = 0;
	/*
	WVector3 entryPoint;
	//去除-0.0f的可能
	r.direction.v[0] = r.direction.v[0] == 0.0f ? 0.0f : r.direction.v[0];
	r.direction.v[1] = r.direction.v[1] == 0.0f ? 0.0f : r.direction.v[1];
	r.direction.v[2] = r.direction.v[2] == 0.0f ? 0.0f : r.direction.v[2];
	
	if (sceneBox.isInBoxInclusive(r.point))
		entryPoint = r.point;
	else
	{
		float tMin, tMax;
		if (sceneBox.isIntersect(r, tMin, tMax))
		{
			//找到与场景包围盒的交点
//			tMin = max(tMin, 0.0f);
			entryPoint = r.point + tMin* r.direction;
		} 
		//与场景包围盒不相交，直接返回
		else
			return false;
	}
	int currNodeID = beginNode, bestTriID = -1;
	float bestT = M_INF_BIG;
	while (currNodeID != -1)
	{
//		cout<<"currNodeID:" <<currNodeID<<endl;
		//内部节点
		/*
		glBegin(GL_POINTS);
		glColor3f(0,0,0);
		glVertex3f(entryPoint.x, entryPoint.y, entryPoint.z);
		glEnd();
		if (nodes[currNodeID]->type != WSKDNode::KDN_LEAF)
		{
			WSKDInterior interior = *((WSKDInterior*)nodes[currNodeID]);
			currNodeID = 
				entryPoint.v[interior.type] < interior.splitPlane ? 
				interior.leftChild : interior.rightChild;
		} 
		//叶节点
		else
		{
			WSKDLeaf& leaf = *((WSKDLeaf*)nodes[currNodeID]);
			float farT;
			int nextNodeID = computeExitFace(leaf, r, entryPoint, farT);
			//找出节点内最近的三角形
			for (int ithTri = 0; ithTri < leaf.nTriangles; ithTri++)
			{
//				cout<<"leaf triangleID "<<leaf.triangleIDs[ithTri]<<endl;
//				triangles[pNode->triangleIDs[ithTri]].showVertexCoords();
				float currT = triangles[leaf.triangleIDs[ithTri]].intersectTest(r);
/*				glColor3f(rand()/32767.0f, 1.0f,1.0f);
				triangles[leaf.triangleIDs[ithTri]].draw(false,true);
				if (currT < bestT)
				{
//					triangles[leaf.triangleIDs[ithTri]].draw(false,true);
					bestT = currT;
					bestTriID = leaf.triangleIDs[ithTri];
//					cout<<"update triangleID:"<<bestTriID<<endl;
				}
			}
			//如果交点在节点内部，求交成功
			if (bestT <= farT)
			{
				if (endNode)
					*endNode = currNodeID;
				goto INTERSECT_END;
			}
			//如果交点在节点外，或者没有交点，通过rope进入下一个节点
			else
			{
				currNodeID = nextNodeID;
			}
		}
	}
INTERSECT_END:
	if (bestTriID == -1)
	{
		return false;
	} 
	else
	{
//		cout<<"bestTriID:"<<bestTriID<<endl;
		triangles[bestTriID].intersect(r,DG);
		return true;
	}*/
}

int WSimpleKD::computeExitFace( WSKDLeaf& node, const WRay& r, WVector3& exitPoint, float& farT)
{
	return 0;
	/*
	float tFar[3];
	//当光线方向分量为0.0f(之前已经将-0.0f转换掉)时，确保对应求出的t值为正无穷
	tFar[0] = (node.box[(r.direction.x >= 0.0f) * 3] - r.point.x) / 
		r.direction.x;
	tFar[1] = (node.box[(r.direction.y >= 0.0f) * 3 + 1] - r.point.y) / 
		r.direction.y;
	tFar[2] = (node.box[(r.direction.z >= 0.0f) * 3 + 2] - r.point.z) / 
		r.direction.z;

	//找出最近的轴向
	int bestAxis = tFar[0] < tFar[1] ? 
		(tFar[0] < tFar[2] ? 0 : 2) :
		(tFar[1] < tFar[2] ? 1 : 2);
	farT = tFar[bestAxis];
	exitPoint = r.point + farT * r.direction;
	exitPoint.v[bestAxis] = node.box[(r.direction.v[bestAxis] >= 0.0f) * 3 + bestAxis];

	return node.ropes[bestAxis * 2 + (r.direction.v[bestAxis] < 0.0f)];

/*
	//在box有体积的情况下
	WVector3 invDir = 1.0f / r.direction;
	float bestT;
	if (box.pMin.x < box.pMax.x && box.pMin.y < box.pMax.y && 
		box.pMin.z < box.pMax.z)
	{		
		float tmax = (box.pMax.x - r.point.x) * invDir.x;
		float tmin = (box.pMin.x - r.point.x) * invDir.x;
		bestT = max(tmin, tmax);
		exitFace = (tmax > tmin) ? 0 : 1;
		tmax = (box.pMax.y - r.point.y) * invDir.y;
		tmin = (box.pMin.y - r.point.y) * invDir.y;
		if (max(tmax, tmin) < bestT)
		{
			exitFace = (tmax > tmin) ? 2 : 3;
			bestT = max(tmax, tmin);
		}
		tmax = (box.pMax.z - r.point.z) * invDir.z;
		tmin = (box.pMin.z - r.point.z) * invDir.z;
		if (max(tmax, tmin) < bestT)
		{
			exitFace = (tmax > tmin) ? 4 : 5;
			bestT = max(tmax, tmin);
		}
		exitPoint = r.point + r.direction * bestT;
		return;
	}
	else if(box.pMin.x >= box.pMax.x)
	{
		exitFace = r.direction.x > 0 ? 0 : 1;
		float bestT = (box.pMax.x - r.point.x) * invDir.x;
		exitPoint = r.point + r.direction * bestT;
	}
	else if(box.pMin.y >= box.pMax.y)
	{
		exitFace = r.direction.y > 0 ? 2 : 3;
		float bestT = (box.pMax.y - r.point.y) * invDir.y;
		exitPoint = r.point + r.direction * bestT;
	}
	else if(box.pMin.z >= box.pMax.z)
	{
		exitFace = r.direction.z > 0 ? 4 : 5;
		float bestT = (box.pMax.z - r.point.z) * invDir.z;
		exitPoint = r.point + r.direction * bestT;
	}*/
}

bool WSimpleKD::isOverlap( WBoundingBox& refBox, 
						 WBoundingBox& testBox, 
						 int refFace )
{
	switch (refFace)
	{
	case 0:
		return testBox.pMin.z <= refBox.pMin.z &&
			   testBox.pMin.y <= refBox.pMin.y &&
			   testBox.pMax.z >= refBox.pMax.z &&
			   testBox.pMax.y >= refBox.pMax.y &&
			   refBox.pMax.x == testBox.pMin.x;
		break;
	case 1:
		return testBox.pMin.z <= refBox.pMin.z &&
			testBox.pMin.y <= refBox.pMin.y &&
			testBox.pMax.z >= refBox.pMax.z &&
			testBox.pMax.y >= refBox.pMax.y &&
			refBox.pMin.x == testBox.pMax.x;
		break;
	case 2:
		return testBox.pMin.z <= refBox.pMin.z &&
			testBox.pMin.x <= refBox.pMin.x &&
			testBox.pMax.z >= refBox.pMax.z &&
			testBox.pMax.x >= refBox.pMax.x &&
			refBox.pMax.y == testBox.pMin.y;
		break;
	case 3:
		return testBox.pMin.z <= refBox.pMin.z &&
			testBox.pMin.x <= refBox.pMin.x &&
			testBox.pMax.z >= refBox.pMax.z &&
			testBox.pMax.x >= refBox.pMax.x &&
			refBox.pMin.y == testBox.pMax.y;
		break;
	case 4:
		return testBox.pMin.y <= refBox.pMin.y &&
			testBox.pMin.x <= refBox.pMin.x &&
			testBox.pMax.y >= refBox.pMax.y &&
			testBox.pMax.x >= refBox.pMax.x &&
			refBox.pMax.z == testBox.pMin.z;
		break;
	case 5:
		return testBox.pMin.y <= refBox.pMin.y &&
			testBox.pMin.x <= refBox.pMin.x &&
			testBox.pMax.y >= refBox.pMax.y &&
			testBox.pMax.x >= refBox.pMax.x &&
			refBox.pMin.z == testBox.pMax.z;
		break;
	}
	return false;
}

int WSimpleKD::checkFace( WBoundingBox& refBox, int& rope , int refFace)
{
	return 0;
	/*
	//对于指向空的rope，直接返回
	if (rope == -1)
		return rope;
	int currNode = rope;
//	printf("currRope: %d\n", rope);
	while (1)
	{
		//到达叶节点，结束搜索
		if (nodes[currNode]->type == WSKDNode::KDN_LEAF)
		{
			return rope;
		} 
		else
		{
			WSKDInterior* pNode = (WSKDInterior*)nodes[currNode];
			int nOverlap = 0;
			//检查跟左节点有没有重合的面
			if (isOverlap(refBox, nodeBoxes[pNode->leftChild], refFace))
			{
				nOverlap++;		currNode = pNode->leftChild;
			} 
			//检查跟右节点有没有重合的面
			if (isOverlap(refBox, nodeBoxes[pNode->rightChild], refFace))
			{
				nOverlap++;		currNode = pNode->rightChild;
			}
			//如果跟两个节点都重合，说明有一个节点的体积为0
			if (nOverlap == 2)
			{
				if ((refFace & 0x01) == 1)
				{
					currNode = pNode->rightChild;
				} 
				else
				{
					currNode = pNode->rightChild;
				}
			}
			//如果两个子节点都没有完全重合的面，直接返回，不修改rope
			if (nOverlap == 0)
				return rope;
			//否则把rope修改为子节点的索引
			rope = currNode;
		}

//		printf("newRope: %d\n", rope);
	}*/
}

void WSimpleKD::drawTriangles()
{
	WSKDLeaf* pNode;
	for (pNode = kdLeafs; pNode < kdLeafs + numLeafs; ++pNode)
	{
		WTriangle* pTri = *pNode->triangle;
		WTriangle* pEnd = pTri + pNode->nTriangles;
		for(; pTri < pEnd; ++pTri)
			pTri->draw();
	}
}

void WSimpleKD::buildExtendedRopes()
{/*
	vector<WSKDNode*>::iterator pNode;
	int ithNode = 0;
	printf("nodeBoxes: %d\n", nodeBoxes.size());
	for (pNode = nodes.begin(); pNode != nodes.end(); pNode++, ithNode++)
	{
		if ((*pNode)->type == WSKDNode::KDN_LEAF)
		{
			WSKDLeaf* pLeaf = (WSKDLeaf*)(*pNode);
			checkFace(nodeBoxes[ithNode], pLeaf->ropes[0], 0);
			checkFace(nodeBoxes[ithNode], pLeaf->ropes[1], 1);
			checkFace(nodeBoxes[ithNode], pLeaf->ropes[2], 2);
			checkFace(nodeBoxes[ithNode], pLeaf->ropes[3], 3);
			checkFace(nodeBoxes[ithNode], pLeaf->ropes[4], 4);
			checkFace(nodeBoxes[ithNode], pLeaf->ropes[5], 5);
		}
	}*/
}

void WSimpleKD::displayNodeInfo()
{
	int maxLeafTris = 0;
	for (WSKDLeaf* pLeaf = kdLeafs; pLeaf < kdLeafs + numLeafs; ++pLeaf)
		maxLeafTris = max(maxLeafTris, pLeaf->nTriangles);
	printf("number of interiors:%d\n"\
		   "number of leafs    :%d\n"\
		   "number of leaf tris:%d\n"\
		   "average leaf tris  :%f\n"\
		   "maximum leaf tris  :%d\n"\
		   "tree depth         :%d\n"\
		   , numInteriors, numLeafs, numLeafTriangles, (float)numLeafTriangles / numLeafs, maxLeafTris,treeDepth);
}