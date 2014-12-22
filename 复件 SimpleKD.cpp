#include "StdAfx.h"
#include "SimpleKD.h"

WSimpleKD::WSimpleKD(void)
{
	nodeMaxDepth = 0;
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
//	sceneBox.pMin += WVector3(WBoundingBox::delta);
//	sceneBox.pMax -= WVector3(WBoundingBox::delta);
	WClock timer;
	timer.begin();


	//新建bounding edge数组，用于基本体的排序
	//由于KD树的划分平面在 XYZ 3个方向上，
	//所以bounding edge数组有3个
	vector<BoundingEdge> edgesX, edgesY, edgesZ;
	edgesX.reserve(totalTriangles * 2);
	edgesY.reserve(totalTriangles * 2);
	edgesZ.reserve(totalTriangles * 2);

	int* triangleMark = new int[totalTriangles];

	//初始化bounding edge 和三角形索引
	for (unsigned int nthTriangle = 0;
		nthTriangle < totalTriangles;
		nthTriangle++)
	{
		WBoundingBox triangleBox = 
			WBoundingBox(triangles[nthTriangle], false);

		addEdge(edgesX, triangleBox.pMin.x, triangleBox.pMax.x, nthTriangle);
		addEdge(edgesY, triangleBox.pMin.y, triangleBox.pMax.y, nthTriangle);
		addEdge(edgesZ, triangleBox.pMin.z, triangleBox.pMax.z, nthTriangle);
	}

	//对bounding edge进行排序
	sort(edgesX.begin(), edgesX.end());
	sort(edgesY.begin(), edgesY.end());
	sort(edgesZ.begin(), edgesZ.end());

	//预留节点空间
	nodes.reserve(totalTriangles * 6);
	nodeBoxes.reserve(totalTriangles * 6);

	//递归构建树
	buildTreeKernel(edgesX, edgesY, edgesZ, 
					sceneBox, triangleMark, totalTriangles, 0);

	//构建ropes
	cout<<"nNodes: "<<nodes.size() << "depth" << nodeMaxDepth <<endl;
	buildBasicRopes(0, -1, -1, -1, -1, -1, -1);
//	buildExtendedRopes();
	timer.end();
	cout<<"build complete.Total time: "<<endl;
	timer.display();
	delete[] triangleMark;
}
void WSimpleKD::addEdge( vector<BoundingEdge>& edges, 
					   float minT, float maxT, 
					   int ithTriangle )
{
	if (minT == maxT)
	{
		BoundingEdge planarEdge;
		planarEdge.type = BoundingEdge::BE_PLANAR;
		planarEdge.t = minT;
		planarEdge.triangleID = ithTriangle;
		edges.push_back(planarEdge);
	} 
	else
	{
		BoundingEdge startEdge, endEdge;
		startEdge.type = BoundingEdge::BE_START;
		endEdge.type = BoundingEdge::BE_END;
		startEdge.triangleID = endEdge.triangleID = ithTriangle;
		startEdge.t = minT; endEdge.t = maxT;
		edges.push_back(startEdge);
		edges.push_back(endEdge);
	}
}
void WSimpleKD::clearTree()
{
	triangles = NULL;
	vector<WSKDNode*>::iterator pNode;
	for (pNode = nodes.begin(); pNode != nodes.end();
		pNode++)
	{
		delete *pNode;
	}
	nodes.clear();
	totalTriangles = 0;
	nodeMaxDepth = 0;
}

void WSimpleKD::buildTreeKernel( vector<BoundingEdge>& edgesX, 
							   vector<BoundingEdge>& edgesY, 
							   vector<BoundingEdge>& edgesZ, 
							   WBoundingBox bBox, 
							   int* triangleMark, int nTriangles, int depth )
{
/*	printf("\n\n##################################################################\nnthNode: %d\nnTriangles: %d\ndepth: %d\n", 
			nodes.size(), nTriangles, depth);*/
	//记录树的最大深度
	if (depth > nodeMaxDepth)
	{
		nodeMaxDepth = depth;
	}
	//若三角形数量少于规定值，新建叶节点
	if (nTriangles <= WSKDNode::maxTrianglesPerLeaf)
	{
		buildLeaf(bBox, edgesX);
		return;
	}
	//否则新建内部节点
	//找出坐标范围最大的轴向分割
	WVector3 deltaBox = bBox.pMax - bBox.pMin;
	WSKDNode::NodeType splitType = 
		(deltaBox.x > deltaBox.y)?
		(deltaBox.x > deltaBox.z? WSKDNode::KDN_XSPLIT:WSKDNode::KDN_ZSPLIT):
		(deltaBox.y > deltaBox.z? WSKDNode::KDN_YSPLIT:WSKDNode::KDN_ZSPLIT);

	int bestPosition, nTriangles_L, nTriangles_R;
	float bestT;
	bool isLeft;
	WBoundingBox box_L = bBox, box_R = bBox;
	vector<BoundingEdge> edgesX_L, edgesY_L, edgesZ_L, 
						 edgesX_R, edgesY_R, edgesZ_R;

	WSKDInterior* interior = new WSKDInterior;
	float tmin, tmax;

//	printf("---------------------------begin to compute split plane\n");
	//计算分割平面的最佳位置
	int ithTest;
	for(ithTest = 0; ithTest < 3; ithTest++)
	{
		tmin = bBox.pMin.v[splitType]; 
		tmax = bBox.pMax.v[splitType];
		switch (splitType)
		{
		case WSKDNode::KDN_XSPLIT:
			computeBestSplit(edgesX, splitType, bBox, 
							nTriangles, nTriangles_L, nTriangles_R,
							bestPosition, isLeft, bestT);
			break;
		case WSKDNode::KDN_YSPLIT:
			computeBestSplit(edgesY, splitType, bBox, 
							nTriangles, nTriangles_L, nTriangles_R,
							bestPosition, isLeft, bestT);
			break;
		case WSKDNode::KDN_ZSPLIT:
			computeBestSplit(edgesZ, splitType, bBox, 
							nTriangles, nTriangles_L, nTriangles_R,
							bestPosition, isLeft, bestT);
		}
		//  以下为无效划分,换一个轴向
		//	1. 分隔平面在bbox上面，且分隔后有一边三角形数目不变
		//	2. 分隔后左右三角形数目都不减少
		if((nTriangles_L == nTriangles && nTriangles_R ==nTriangles) ||
				((bestT == tmin || bestT == tmax)&&
				(nTriangles_L == nTriangles || nTriangles_R == nTriangles)))
		{
//			printf("----------next axis\n");
			splitType = WSKDNode::NodeType((splitType + 1) % 3);
		}
		else 
			break;
	}
	//无满意划分，新建叶节点
	if (ithTest == 3)
	{
		printf("no proper division.\n");
		buildLeaf(bBox, edgesX);
		return;
	}
/*
	printf("nTriangles_L: %d, nTriangles_R: %d\n",nTriangles_L,nTriangles_R);
	printf("bestT: %1.3f\tbestBE:%d\tisLeft:%d\n",bestT,bestPosition,(int)isLeft);
	printf("---------------------------begin to mark triangle\n");
*/
	vector<int> middleTriangleID;
	vector<BoundingEdge> middleEdgeX_L, middleEdgeY_L, middleEdgeZ_L,
						 middleEdgeX_R, middleEdgeY_R, middleEdgeZ_R;

	//根据已有的分隔平面位置，标记三角形
	//只在左节点的三角形对应标记为 2
	//只在左节点的三角形对应标记为-2
	//横跨分隔平面的三角形对应标记为 0
	interior->type = splitType;
	switch (splitType)
	{
	case WSKDNode::KDN_XSPLIT:
//		printf("Xsplit\n");
		markTriangles(edgesX, middleTriangleID, bestPosition, 
			nTriangles, triangleMark, isLeft);
		break;
	case WSKDNode::KDN_YSPLIT:
//		printf("Ysplit\n");
		markTriangles(edgesY, middleTriangleID, bestPosition, 
			nTriangles, triangleMark, isLeft);
		break;
	case WSKDNode::KDN_ZSPLIT:
//		printf("Zsplit\n");
		markTriangles(edgesZ, middleTriangleID, bestPosition, 
			nTriangles, triangleMark, isLeft);
	}
	box_L.pMax.v[splitType] = bestT;
	box_R.pMin.v[splitType] = bestT;
/*
	printf("overlapping triangles: %d\n",middleTriangleID.size());
	printf("---------------------------begin to compute new BE\n");
*/
	//创建新的BE，对于左右节点，新的BE数量都是一样的
	getNewSortedBE(middleTriangleID, box_L, middleEdgeX_L, middleEdgeY_L, middleEdgeZ_L);
	getNewSortedBE(middleTriangleID, box_R, middleEdgeX_R, middleEdgeY_R, middleEdgeZ_R);

	
/*
	printf("middleEdgeX_L: %d\tmiddleEdgeY_L: %d\tmiddleEdgeZ_L: %d\n",
		middleEdgeX_L.size(),middleEdgeY_L.size(),middleEdgeZ_L.size());
	printf("middleEdgeX_R: %d\tmiddleEdgeY_R: %d\tmiddleEdgeZ_R: %d\n",
		middleEdgeX_R.size(),middleEdgeY_R.size(),middleEdgeZ_R.size());
	printf("---------------------------begin to merge BE\n");
*/
	//对三个方向的BE进行合并
	//合并后对于每个子节点三个方向的BE数量都是一样的，
	//实际上就是节点内部的三角形的包围盒坐标值
	mergeBE(edgesX, middleEdgeX_L, middleEdgeX_R, edgesX_L, edgesX_R, 
		nTriangles_L, nTriangles_R, triangleMark);
	mergeBE(edgesY, middleEdgeY_L, middleEdgeY_R, edgesY_L, edgesY_R, 
		nTriangles_L, nTriangles_R, triangleMark);
	mergeBE(edgesZ, middleEdgeZ_L, middleEdgeZ_R, edgesZ_L, edgesZ_R,  
		nTriangles_L, nTriangles_R, triangleMark);

/*
	edgesX.clear();
	edgesY.clear();
	edgesZ.clear();
	middleEdgeX_L.clear();
	middleEdgeX_R.clear();
	middleEdgeY_L.clear();
	middleEdgeY_R.clear();
	middleEdgeZ_L.clear();
	middleEdgeZ_R.clear();
	middleTriangleID.clear();*/

/*
	printf("bestPos: %d\nsplitT: %1.0f\n", bestPosition, bestT);
	printf("edgesX_L: %d\tedgesY_L: %d\tedgesZ_L: %d\n",
		edgesX_L.size(),edgesY_L.size(),edgesZ_L.size());
	printf("edgesX_R: %d\tedgesY_R: %d\tedgesZ_R: %d\n",
		edgesX_R.size(),edgesY_R.size(),edgesZ_R.size());
*/
	interior->splitPlane = bestT;
	nodes.push_back(interior);
	nodeBoxes.push_back(bBox);
	interior->leftChild = nodes.size();
	buildTreeKernel(edgesX_L, edgesY_L, edgesZ_L,
				box_L, triangleMark, nTriangles_L, depth+1);
	interior->rightChild = nodes.size();
	buildTreeKernel(edgesX_R, edgesY_R, edgesZ_R,
				box_R, triangleMark, nTriangles_R, depth+1);
}

void WSimpleKD::computeBestSplit( 
			vector<BoundingEdge>& edges, 
			WSKDNode::NodeType splitType, 
			WBoundingBox& bBox, 
			int nTriangles,
			int& nTriangles_L,
			int& nTriangles_R,
			int& bestPosition,
			bool& isLeft,
			float& bestT)
{
	//分隔平面的范围
	float tmin, tmax;

	//根据splitType选择方向
	tmin = bBox.pMin.v[splitType];
	tmax = bBox.pMax.v[splitType];

	//选择最佳的分隔平面位置
	int nthBE = 0;
	int bestBE = 0;
	float emptyFactor, cost;
	float bestcost = FLT_MAX;
	//nBelow ， nAbove代表两边的三角形数量
	float nTempBelow = 0.0f, nAbove = (float)nTriangles, nBelow = 0.0f;
	float lengthL, lengthR;
	vector<BoundingEdge>::iterator pBE;
//	printf("minT: %1.3f \t maxT:%1.3f\n",tmin,tmax);
	for (pBE = edges.begin(); pBE != edges.end();
		pBE++, nthBE++)
	{
		//计算分隔平面左右两侧的长度
		lengthL = (pBE->t) - tmin;
		lengthR = tmax - (pBE->t);
		if(pBE->type != BoundingEdge::BE_PLANAR)
		{
			//计算分割平面两侧的三角形数量
			//实际推算发现，实际的三角形数量应该是nTempBelow的上一个值
			nBelow = nTempBelow;
			if (pBE->type == BoundingEdge::BE_START)
				nTempBelow++;
			else
				nAbove--;
			//计算比例因数，开销函数
			emptyFactor = computeEmptyFtr(nBelow, nAbove, nTriangles, pBE->t, tmin, tmax);
			cost = computeCost(emptyFactor, lengthL, lengthR, nBelow, nAbove);
			//如果找到更小的开销，更新相关参数
			if (cost < bestcost)
			{
	/*   		printf("BE-t : %1.3f\t TriID: %d\t cost: %1.0f\t nthBE:%d\n",
						pBE->t,pBE->triangleID, cost, nthBE);*/
				updateBest(bestBE, nthBE, bestcost, cost, 
					nTriangles_L, nTriangles_R, (int)nBelow, (int)nAbove);
			}
		}
		else
		{
			//对于 BE_PLANAR 类型的 BE， 分别计算所含三角形在分隔平面左边和右边的情况
			//三角形在右边的情况，相当于遇到一个开始BE
			nBelow = nTempBelow; nTempBelow++;
			//计算比例因数，开销函数
			emptyFactor = computeEmptyFtr(nBelow, nAbove, nTriangles, pBE->t, tmin, tmax);
			cost = computeCost(emptyFactor, lengthL, lengthR, nBelow, nAbove);

			//如果找到更小的开销，更新相关参数
			if (cost < bestcost)
			{
/*   		printf("BE-t : %1.3f\t TriID: %d\t cost: %1.0f\t nthBE:%d\n",
					pBE->t,pBE->triangleID, cost, nthBE);*/
				updateBest(bestBE, nthBE, bestcost, cost, 
					nTriangles_L, nTriangles_R, (int)nBelow, (int)nAbove);
				isLeft = false;
			}

			//三角形在左边的情况，相当于遇到一个结束BE
			nBelow = nTempBelow; nAbove--;
			//计算比例因数，开销函数
			emptyFactor = computeEmptyFtr(nBelow, nAbove, nTriangles, pBE->t, tmin, tmax);
			cost = computeCost(emptyFactor, lengthL, lengthR, nBelow, nAbove);

			//如果找到更小的开销，更新相关参数
			if (cost < bestcost)
			{
/*   		printf("BE-t : %1.3f\t TriID: %d\t cost: %1.0f\t nthBE:%d\n",
				pBE->t,pBE->triangleID, cost, nthBE);*/
				updateBest(bestBE, nthBE, bestcost, cost, 
					nTriangles_L, nTriangles_R, (int)nBelow, (int)nAbove);
				isLeft = true;
			}
		}

	}
//	printf("bestBE: %d\n",bestBE);
	bestPosition = bestBE;
	bestT = edges[bestBE].t;
}

void WSimpleKD::markTriangles( 
			vector<BoundingEdge>& refEdges, 
			vector<int>& middleTriangleID,
			int bestBE,
			int nTriangles,
			int* triangleMark,
			bool isLeft)
{
	//记录横跨两个子节点的三角形索引
	middleTriangleID.reserve(nTriangles);

	vector<BoundingEdge>::iterator pRefEdges,pTarEdges1,pTarEdges2;
	BoundingEdge splitEdge = refEdges[bestBE];

	//确定三角形会分到哪个节点里面
	//同一个三角形的BE必然先出现start，再出现end
	//循环后，
	//只在左边的三角形对应标记为 2
	//只在右边的三角形对应标记为-2
	//横跨节点的三角形对应标记为 0
	int ithEdge = 0;
	for (pRefEdges = refEdges.begin();
		pRefEdges != refEdges.end(); pRefEdges++, ithEdge++)
	{
/*		printf("triangleID: %d\tedgeType:%d\tT:%3.23f",pRefEdges->triangleID,pRefEdges->type,pRefEdges->t);*/
		//对于开始边
		if (pRefEdges->type == BoundingEdge::BE_START)
			if (ithEdge < bestBE)
			{
				//在左边
//				printf(" left\tbegin\n");
				triangleMark[pRefEdges->triangleID] = 1;
			}
			else
			{
//				printf(" right\tbegin\n");
				//在右边，说明整个三角形在分割平面的右边
				triangleMark[pRefEdges->triangleID] = -1;
			}
		//对于结束边
		else if (pRefEdges->type == BoundingEdge::BE_END)
			if (ithEdge <= bestBE)
			{
//				printf(" left\tend\n");
				//在左边， 说明整个三角形在分割平面的左边
				triangleMark[pRefEdges->triangleID] += 1;
			}
			else
			{
				//在右边
//				printf(" right\tend\n");
				triangleMark[pRefEdges->triangleID] -= 1;
				//把横跨节点的三角形记录下来
				if (triangleMark[pRefEdges->triangleID] == 0)
					middleTriangleID.push_back(pRefEdges->triangleID);
			}
		//对于平面边根据它的t参数以及isLeft就可以确定在分割平面的哪一边
		else
		{	//printf("\n");
			if (ithEdge < bestBE)
				triangleMark[pRefEdges->triangleID] = 2;
			else if (ithEdge > bestBE)
				triangleMark[pRefEdges->triangleID] = -2;
			else
				triangleMark[pRefEdges->triangleID] = isLeft ? 2 : -2;
		}
	}

	for (pRefEdges = refEdges.begin();
		pRefEdges != refEdges.end(); pRefEdges++)
	{
		//打印有问题的标记
		if (triangleMark[pRefEdges->triangleID] != -2 &&
			triangleMark[pRefEdges->triangleID] != 0 &&
			triangleMark[pRefEdges->triangleID] != 2)
			printf("***************wrong triangleMark: %d, triangleID: %d\n",
				triangleMark[pRefEdges->triangleID],pRefEdges->triangleID);
	}
/*	for (int i = 0; i< middleTriangleID.size(); i++)
		printf("overlapping ID: %d\n",middleTriangleID[i]);*/
}

void WSimpleKD::drawTree( unsigned int nthBox/*=0*/, float R /*= 0.7*/, float G /*= 0.7*/, float B /*= 0.7*/ )
{
//	cout<<"\nnode size:"<<nodes.size()<<endl;
	glColor3f(R, G, B);
	drawTreeRecursive(0, sceneBox);
}

void WSimpleKD::drawTreeRecursive( int nthNode, const WBoundingBox& box )
{
	box.draw();
	if (!nodes.size())
		return;
	WBoundingBox leftBox, rightBox;
	leftBox = rightBox = box;
	WSKDNode* node = nodes[nthNode];
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
//	cout<<"left child: "<<((WSKDInterior*)node)->leftChild<<'\n'
//		<<"right child:"<<((WSKDInterior*)node)->rightChild<<endl;
	drawTreeRecursive(((WSKDInterior*)node)->leftChild, leftBox);
	drawTreeRecursive(((WSKDInterior*)node)->rightChild, rightBox);
}


void WSimpleKD::buildLeaf( WBoundingBox bBox, vector<BoundingEdge>& edges )
{
	WSKDLeaf* leaf = new WSKDLeaf;
	leaf->box[0] = bBox.pMin.x;
	leaf->box[1] = bBox.pMin.y;
	leaf->box[2] = bBox.pMin.z;
	leaf->box[3] = bBox.pMax.x;
	leaf->box[4] = bBox.pMax.y;
	leaf->box[5] = bBox.pMax.z;
	leaf->type = WSKDNode::KDN_LEAF;
	int nthTriangle = 0;
	for (unsigned int ithEdge = 0; 
		  ithEdge<edges.size(); 
		  ithEdge++)
	{
		if (edges[ithEdge].type == BoundingEdge::BE_START ||
			edges[ithEdge].type == BoundingEdge::BE_PLANAR)
		{
//			printf("leaf triangle ID: %d\n",edges[ithEdge].triangleID);
			leaf->triangleIDs[nthTriangle] = 
				edges[ithEdge].triangleID;
			nthTriangle = min(nthTriangle+1, WSKDNode::maxTrianglesPerLeaf-1);
		}
	}
	leaf->nTriangles = nthTriangle;
	nodes.push_back(leaf);
	nodeBoxes.push_back(bBox);
}

void WSimpleKD::getNewSortedBE( vector<int>& middleTriangleID,
						const WBoundingBox& clipBox, 
						vector<BoundingEdge>& middleBEX, 
						vector<BoundingEdge>& middleBEY, 
						vector<BoundingEdge>& middleBEZ )
{
	//预留空间
	int nNewEdges = middleTriangleID.size() * 2;
	middleBEX.reserve(nNewEdges);
	middleBEY.reserve(nNewEdges);
	middleBEZ.reserve(nNewEdges);

	//设置用于剪切的长方体
//	clipBox.displayCoords();
	WBoxClipper clipper(clipBox);
	BoundingEdge startEdge, endEdge;
	startEdge.type = BoundingEdge::BE_START;
	endEdge.type = BoundingEdge::BE_END;

	vector<int>::iterator pTriID;
	for(pTriID = middleTriangleID.begin();
		pTriID != middleTriangleID.end(); pTriID++)
	{
		WTriangle tri;
		tri.point1 = triangles[*pTriID].point1;
		tri.point2 = triangles[*pTriID].point2;
		tri.point3 = triangles[*pTriID].point3;

		//获得剪切后对应的包围盒
		WBoundingBox box;
		if (/*clipper.getClipTriangleBox(tri, box)*/1)
		{
			//若剪切还有剩余，新建一组BE
			addEdge(middleBEX, box.pMin.x, box.pMax.x, *pTriID);
			addEdge(middleBEY, box.pMin.y, box.pMax.y, *pTriID);
			addEdge(middleBEZ, box.pMin.z, box.pMax.z, *pTriID);
		}
		else
		{
			//显示没有相交的情况
			//正常情况下三角形都跟包围盒相交，不会有这种情况
			cout<<"#####WTriangle#####"<<endl;
			printf("TriangleID: %d\n",*pTriID);
			tri.showVertexCoords();
			clipBox.displayCoords();
		}
	}
	//对边排序
	sort(middleBEX.begin(), middleBEX.end());
	sort(middleBEY.begin(), middleBEY.end());
	sort(middleBEZ.begin(), middleBEZ.end());
}

void WSimpleKD::mergeBE( vector<BoundingEdge>& oldEdge, 
					   vector<BoundingEdge>& middleEdge_L,
					   vector<BoundingEdge>& middleEdge_R,
					   vector<BoundingEdge>& newEdge_L,
					   vector<BoundingEdge>& newEdge_R,
					   int nTriangles_L, int nTriangles_R, 
					   int* triangleMark )
{
	newEdge_L.clear();newEdge_R.clear();
	newEdge_L.reserve(nTriangles_L * 2);
	newEdge_R.reserve(nTriangles_R * 2);

	vector<BoundingEdge>::iterator pOldEdge, pMiddleEdge_L, pMiddleEdge_R,
								   pNewEdge_L, pNewEdge_R;
	
	pOldEdge = oldEdge.begin();
	pMiddleEdge_L = middleEdge_L.begin();
	pMiddleEdge_R = middleEdge_R.begin();

	bool isOldTerminated ,isLeftTerminated, isRightTerminated;
	isOldTerminated = isLeftTerminated = isRightTerminated = false;
	if (middleEdge_L.size() == 0)
		isLeftTerminated = true;
	if (middleEdge_R.size() == 0)
		isRightTerminated = true;
	if (oldEdge.size() == 0)
		isOldTerminated = true;

	//合并新创建的边（middleEdge）和原有的边（oldEdge）
	//对于分隔轴向，在按顺序访问原来的BE时，
	//必然是先出现对应三角形标记为2的BE，再出现标记为-2的BE
	//但是对于其他轴线，则没有这个规律，在左节点的BE可能由于t值较大排在在右节点的BE后面
	//因此，对于左右节点都需要对原来的BE从头到尾扫描

	//创建左节点的BE
	for (int nthLeftEdge = 0; nthLeftEdge < nTriangles_L * 2; nthLeftEdge++)
	{
		//跳过无用的节点
		if(!isOldTerminated)
		{	
			while (triangleMark[pOldEdge->triangleID] != 2 )
			{
//				printf("triangleMark: %d\n",triangleMark[pOldEdge->triangleID]);
				if (pOldEdge + 1 != oldEdge.end())
					++pOldEdge;
				else
				{
					isOldTerminated = true;
					break;
				}
			}
		}
		//如果oldEdge和middleEdge都没有终止，加入较小的一个
		if (!isOldTerminated && !isLeftTerminated)
			if(*pOldEdge < *pMiddleEdge_L)
			{
				newEdge_L.push_back(*pOldEdge);
				if (pOldEdge + 1 != oldEdge.end())
					++pOldEdge;
				else
					isOldTerminated = true;
			}
			else
			{
				newEdge_L.push_back(*pMiddleEdge_L);
				if (pMiddleEdge_L + 1 != middleEdge_L.end())
					++pMiddleEdge_L;
				else
					isLeftTerminated = true;
			}
		//如果middleEdge终止了，但是oldEdge没终止，加入oldEdge
		else if (!isOldTerminated)
		{
			newEdge_L.push_back(*pOldEdge);
			if (pOldEdge + 1 != oldEdge.end())
				++pOldEdge;
			else
				isOldTerminated = true;
		}
		//如果oldEdge终止了，但是middleEdge没终止，加入middleEdge
		else if (!isLeftTerminated)
		{
			newEdge_L.push_back(*pMiddleEdge_L);
			if (pMiddleEdge_L + 1 != middleEdge_L.end())
				++pMiddleEdge_L;
			else
				isLeftTerminated = true;
		}
	}
	//创建右节点的BE
	pOldEdge = oldEdge.begin();
	isOldTerminated = false;
	if (oldEdge.size() == 0)
		isOldTerminated = true;
	for (int nthRightEdge = 0; nthRightEdge < nTriangles_R * 2; nthRightEdge++)
	{
		//跳过无用的节点
		if (!isOldTerminated)
		{
			while (triangleMark[pOldEdge->triangleID] != -2 )
			{
				if (pOldEdge + 1 != oldEdge.end())
					++pOldEdge;
				else
				{
					isOldTerminated = true;
					break;
				}
			}
		}
		//如果oldEdge和middleEdge都没有终止，加入较小的一个
		if (!isOldTerminated && !isRightTerminated)
			if(*pOldEdge < *pMiddleEdge_R)
			{
				newEdge_R.push_back(*pOldEdge);
				if (pOldEdge + 1 != oldEdge.end())
					++pOldEdge;
				else
					isOldTerminated = true;
			}
			else
			{
				newEdge_R.push_back(*pMiddleEdge_R);
				if (pMiddleEdge_R + 1 != middleEdge_R.end())
					++pMiddleEdge_R;
				else
					isRightTerminated = true;
			}
		//如果middleEdge终止了，但是oldEdge没终止，加入oldEdge
		else if (!isOldTerminated)
		{
			newEdge_R.push_back(*pOldEdge);
			if (pOldEdge + 1 != oldEdge.end())
				++pOldEdge;
			else
				isOldTerminated = true;
		}
		//如果oldEdge终止了，但是middleEdge没终止，加入middleEdge
		else if (!isRightTerminated)
		{
			newEdge_R.push_back(*pMiddleEdge_R);
			if (pMiddleEdge_R + 1 != middleEdge_R.end())
				++pMiddleEdge_R;
			else
				isRightTerminated = true;
		}
	}
	
/*	printf("###########################\n");
	for (pOldEdge = oldEdge.begin();
		pOldEdge != oldEdge.end(); pOldEdge++)
	{
		printf(" triangleOldID: %d\t triangleMark:%d\n",pOldEdge->triangleID,triangleMark[pOldEdge->triangleID]);
	}*/
/*
	for (pNewEdge_L = newEdge_L.begin();
		pNewEdge_L != newEdge_L.end(); pNewEdge_L++)
	{
		printf(" triangleID_L: %d\t triangleMark:%d\t edgeType:%d\n",
			pNewEdge_L->triangleID,triangleMark[pNewEdge_L->triangleID],
			(int)pNewEdge_L->type);
	}
	for (pNewEdge_R = newEdge_R.begin();
		pNewEdge_R != newEdge_R.end(); pNewEdge_R++)
	{
		printf(" triangleID_R: %d\t triangleMark:%d\t edgeType:%d\n",
			pNewEdge_R->triangleID,triangleMark[pNewEdge_R->triangleID],
			(int)pNewEdge_R->type);
	}*/
}

void WSimpleKD::buildBasicRopes(int ithNode, 
							   int ropeX_P, int ropeX_N,
							   int ropeY_P, int ropeY_N,
							   int ropeZ_P, int ropeZ_N)
{
	WSKDNode* pNode = nodes[ithNode];
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


bool WSimpleKD::BoundingEdge::operator<( const BoundingEdge&e ) const
{
	//不同位置, t值较小的较前
	if (t < e.t)
		return true;
	if (t > e.t)
		return false;
	//以下为 t 相等时候的情况
	//两个 t 相等时，确保 planar类型的最前，start类型的较前， end 类型的较后
	//这样做确保在标记三角形的时候先遇到
	//一个三角形的start边，再遇到end边
	if (type < e.type)
		return true;
	if (type > e.type)
		return false;
	//两个 t 相等, 类型相等的BE
	//按照triangleID排序
	return triangleID < e.triangleID;
}

bool WSimpleKD::isIntersect( WRay& r, int beginNode)
{
	//	printf("************************\n");
	WVector3 entryPoint;
//	beginNode = 0;
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
		glEnd();*/
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
				triangles[leaf.triangleIDs[ithTri]].draw(false,true);*/
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

}

bool WSimpleKD::intersect( WRay& r,WDifferentialGeometry& DG, 
						 int* endNode, int beginNode)
{
	//	printf("************************\n");
//	beginNode = 0;
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
		glEnd();*/
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
				triangles[leaf.triangleIDs[ithTri]].draw(false,true);*/
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
	}
}

int WSimpleKD::computeExitFace( WSKDLeaf& node, const WRay& r, WVector3& exitPoint, float& farT)
{
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
	}
}

void WSimpleKD::drawTriangles()
{
	vector<WSKDNode*>::iterator pNode;
	for (pNode = nodes.begin(); pNode != nodes.end(); ++pNode)
	{
		if ((*pNode)->type == WSKDNode::KDN_LEAF)
		{
			WSKDLeaf* pLeaf = (WSKDLeaf*)(*pNode);
			for (int ithTriangle = 0; ithTriangle < pLeaf->nTriangles; 
				ithTriangle++)
			{
				triangles[pLeaf->triangleIDs[ithTriangle]].draw();
			}
		}
	}
}

void WSimpleKD::buildExtendedRopes()
{
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
	}
}