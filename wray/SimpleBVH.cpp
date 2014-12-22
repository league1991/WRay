#include "StdAfx.h"
#include "SimpleBVH.h"

WSimpleBVH::WSimpleBVH(void)
{
	triangles = NULL;
	nTriangles = 0;
	compressedNodes = NULL;
}

WSimpleBVH::~WSimpleBVH(void)
{
	this->clearTree();
}

void WSimpleBVH::buildTree( WScene&scene )
{
	cout<<"begin to build BVH"<<endl;
	scene.getTriangleArray(triangles,nTriangles);
	cout<<"total triangles: "<<nTriangles<<endl;
	sceneBox = scene.getBBox();
	vector<unsigned int>triangleIDs;
	triangleIDs.resize(nTriangles);

	for(unsigned int i=0;
		i<nTriangles;
		i++)
	{
//		cout<<i<<endl;
		triangleIDs[i]=i;
	}
	nodeMaxDepth = 0;
	nodes.clear();
	nodes.reserve(nTriangles * 2);
	buildTreeKernel(triangleIDs,sceneBox);
	buildEnhancedTree();
	cout<<"build complete.\n"<<endl;
	cout<<"node length: "<<nodes.size()<<endl;
	cout<<"node max depth: "<<nodeMaxDepth<<endl;
}

void WSimpleBVH::buildTreeKernel( 
	vector<unsigned int>& triangleIDs, WBoundingBox&box ,
	unsigned int depth,bool isIll)
{
	//检测节点达到的最大深度
	if(depth>nodeMaxDepth)
		nodeMaxDepth = depth;
// 	cout<<"\ntree depth"<<depth<<endl;
//  	cout<<"\n\nnode array size:"<<nodes.size()<<endl;
//  	cout<<"triangle count"<<triangleIDs.size()<<endl;
//  	cout<<"bound box:\n"<<ends;
// 	box.displayCoords();
	unsigned int currNodeID=nodes.size();
	//如果三角形节点数小于设定值
	//或出现病态情况，新建叶节点
	if(triangleIDs.size()<=WBVHNode::maxTrianglesPerLeaf
		||isIll)
	{
//		cout<<"leaf node reached."<<endl;
		WBVHNode node;
		node.type = WBVHNode::BVHN_LEAF;
		node.nTriangles = min(triangleIDs.size(),
							 WBVHNode::maxTrianglesPerLeaf);

		for (char ithTriangle = 0; ithTriangle < WBVHNode::maxTrianglesPerLeaf; ++ithTriangle)
		{
			node.tris[ithTriangle] = ithTriangle < node.nTriangles ? 
				&triangles[triangleIDs[ithTriangle]] : NULL;
		}
		nodes.push_back(node);
		return;
	}
	else
	{
		WBVHNode node;
		node.box[0] = box.pMin.x;
		node.box[1] = box.pMin.y;
		node.box[2] = box.pMin.z;
		node.box[3] = box.pMax.x;
		node.box[4] = box.pMax.y;
		node.box[5] = box.pMax.z;
		nodes.push_back(node);
	}
	//构建质心包围盒
	WBoundingBox centroidBox=
		WBoundingBox(getTriangle(triangleIDs[0]).getCentroid()-
					WVector3(1e-4f),
		getTriangle(triangleIDs[0]).getCentroid()+WVector3(1e-4f));
	for(vector<unsigned int>::iterator triPtr=
		triangleIDs.begin();triPtr!=triangleIDs.end();
		triPtr++)
	{
		centroidBox.merge(getTriangle(*triPtr).getCentroid());
	}

	//包围盒跨度,选择跨度最大的一个轴线来分隔
	WVector3 boxExtent=centroidBox.pMax-centroidBox.pMin;
	char splitAxis=
		boxExtent.x>boxExtent.y?
		(boxExtent.x>boxExtent.z?'x':'z'):
		(boxExtent.y>boxExtent.z?'y':'z');
	//	splitAxis='x';//调试用

	//Bin的数量
	unsigned int nBins=max(4,int(triangleIDs.size()/binRatio));
	//	cout<<"nBins:"<<nBins<<endl;
	//Bins数组
	WBin* bins = new WBin[nBins];
	// 	vector<WBin> bins;
	// 	bins.resize(nBins);
	//每个Bin的坐标间隔
	WVector3 binInterval = boxExtent/float(nBins);
	//	cout<<"binInterval:"<<endl;
//	binInterval.showCoords();

	//左右节点包含的三角形
	vector<unsigned int>leftTriangleIDs,rightTriangleIDs;
	WBoundingBox leftBox,rightBox;

	//记录三角形所在bin的数组
	unsigned int* inNthBin = 
		new unsigned int[triangleIDs.size()];

	//记录各种方案对应左右节点的包围盒面积
	float* SL = new float[nBins-1];
	float* SR = new float[nBins-1];

	//bestCost		开销函数取得的最小值
	//bestScheme	对应的划分方案
	bool isFirst,isLeftFirst,isRightFirst;
	float bestCost;
	float totalCost;
	unsigned int bestScheme;
	unsigned int totalL;
	unsigned int totalR;
	unsigned int nthTriangle;

	vector<unsigned int>::iterator trianglePtr;
	switch (splitAxis)
	{
	case 'x':
		nodes[currNodeID].type = WBVHNode::BVHN_XSPLIT;

		nthTriangle=0;
		//把三角形放进对应的Bin中
		for(trianglePtr=triangleIDs.begin();
			trianglePtr!=triangleIDs.end();
			trianglePtr++,nthTriangle++)
		{
			//计算质心
			WVector3 centroid=
				getTriangle(*trianglePtr).getCentroid();

			//放入对应的bin中
			unsigned int targetBin;
			inNthBin[nthTriangle]=targetBin=
				min(unsigned int((centroid.x-centroidBox.pMin.x)/
				binInterval.x),nBins-1);
			// 			cout<<"targetBin float :"
			// 				<<(centroid.x-centroidBox.pMin.x)/
			// 				binInterval.x<<endl;
			// 			cout<<"targetBin: "<<targetBin<<endl;
			//			cout<<inNthBin[nthTriangle]<<' ';

			if(bins[targetBin].nTriangles==0)
			{
				bins[targetBin].box=
					WBoundingBox(getTriangle(*trianglePtr));
			}
			else
			{
				bins[targetBin].box.merge(getTriangle(*trianglePtr));
			}
			bins[targetBin].nTriangles++;
		}
		break;
	case 'y':
		nodes[currNodeID].type = WBVHNode::BVHN_YSPLIT;

		nthTriangle=0;
		//把三角形放进对应的Bin中
		for(trianglePtr=triangleIDs.begin();
			trianglePtr!=triangleIDs.end();
			trianglePtr++,nthTriangle++)
		{
			//计算质心
			WVector3 centroid=
				getTriangle(*trianglePtr).getCentroid();

			//放入对应的bin中
			unsigned int targetBin;
			inNthBin[nthTriangle]=targetBin=
				min(unsigned int((centroid.y-centroidBox.pMin.y)/
				binInterval.y),nBins-1);
			// 			cout<<"targetBin float :"
			// 				<<(centroid.x-centroidBox.pMin.x)/
			// 				binInterval.x<<endl;
			// 			cout<<"targetBin: "<<targetBin<<endl;
			//			cout<<inNthBin[nthTriangle]<<' ';

			if(bins[targetBin].nTriangles==0)
			{
				bins[targetBin].box=
					WBoundingBox(getTriangle(*trianglePtr));
			}
			else
			{
				bins[targetBin].box.merge(getTriangle(*trianglePtr));
			}
			bins[targetBin].nTriangles++;
		}
		break;
	case 'z':
		nodes[currNodeID].type = WBVHNode::BVHN_ZSPLIT;

		nthTriangle=0;
		//把三角形放进对应的Bin中
		for(trianglePtr=triangleIDs.begin();
			trianglePtr!=triangleIDs.end();
			trianglePtr++,nthTriangle++)
		{
			//计算质心
			WVector3 centroid=
				getTriangle(*trianglePtr).getCentroid();

			//放入对应的bin中
			unsigned int targetBin;
			inNthBin[nthTriangle]=targetBin=
				min(unsigned int((centroid.z-centroidBox.pMin.z)/
				binInterval.z),nBins-1);
			// 			cout<<"targetBin float :"
			// 				<<(centroid.x-centroidBox.pMin.x)/
			// 				binInterval.x<<endl;
			// 			cout<<"targetBin: "<<targetBin<<endl;
			//			cout<<inNthBin[nthTriangle]<<' ';

			if(bins[targetBin].nTriangles==0)
			{
				bins[targetBin].box=
					WBoundingBox(getTriangle(*trianglePtr));
			}
			else
			{
				bins[targetBin].box.merge(getTriangle(*trianglePtr));
			}
			bins[targetBin].nTriangles++;
		}
		break;
	}
	//		cout<<"all bins got."<<endl;

	//对于所有可能的分隔，计算左右节点包围盒的面积
	//一共有nBins-1个分隔方案
	leftBox = rightBox = WBoundingBox(WVector3(0),WVector3(0));
	isLeftFirst = isRightFirst = true;
	for(unsigned int nthBin=0;nthBin<nBins-1;nthBin++)
	{
// 		printf("bins %d has %d triangles\n",
// 			nthBin,bins[nthBin].nTriangles);
		if(bins[nthBin].nTriangles!=0)
		{
			if(isLeftFirst)
			{
				isLeftFirst = false;
				leftBox = bins[nthBin].box;
			}
			else
			{
				leftBox.merge(bins[nthBin].box);
			}
		}
		if(bins[nBins-1-nthBin].nTriangles!=0)
		{
			if(isRightFirst)
			{
				isRightFirst = false;
				rightBox = bins[nBins-1-nthBin].box;
			}
			else
			{
				rightBox.merge(bins[nBins-1-nthBin].box);
			}
		}
		SL[nthBin] = leftBox.halfArea();
		SR[nBins-nthBin-2] = rightBox.halfArea();
		//			cout<<"SL"<<SL[nthBin]<<endl;
	}
// 	printf("bins %d has %d triangles\n",
// 		nBins-1,bins[nBins-1].nTriangles);

	//计算能使开销函数最小的分隔值,并记录对应的数据
	//左右节点包围盒包含的三角形总数
	totalL = bins[0].nTriangles;
	totalR = triangleIDs.size()-totalL;
	//记录最佳的分隔
	bestScheme=0;
	bestCost=FLT_MAX;
	//一共有nBins-1个分隔方案
	unsigned int nthScheme;
	for(nthScheme = 0;
		nthScheme<nBins-1;nthScheme++)
	{
		totalCost = SL[nthScheme]*totalL + SR[nthScheme]*totalR;
		totalL += bins[nthScheme+1].nTriangles;
		totalR -= bins[nthScheme+1].nTriangles;

		if(totalCost<bestCost)
		{
			bestCost = totalCost;
			bestScheme = nthScheme;
//			cout<<"best scheme"<<bestScheme<<endl;
		}
	}
/*	printf("bestCost:%f\nbestScheme%d \n",bestCost,bestScheme);
	cout<<"best scheme found."<<endl;*/
	//计算左右节点包围盒
	//将bins[0]-bins[bestScheme]加入左节点包围盒
	//其余加入右节点包围盒
	isFirst = true;
	for(unsigned int nthBin=0;nthBin<=bestScheme;nthBin++)
	{
		if(bins[nthBin].nTriangles!=0)
		{
			if(isFirst)
			{
				leftBox = bins[nthBin].box;
				isFirst = false;
			}
			else
				leftBox.merge(bins[nthBin].box);
		}
	}
	isFirst = true;
	for(unsigned int nthBin = bestScheme+1;
		nthBin<nBins ;nthBin++)
	{
		if(bins[nthBin].nTriangles!=0)
		{
			if(isFirst)
			{
				rightBox = bins[nthBin].box;
				isFirst = false;
			}
			else
				rightBox.merge(bins[nthBin].box);
		}
	}
//	cout<<"bbox merged"<<endl;
	nthTriangle=0;
	//把三角形放进对应的Bin中
	for(trianglePtr=triangleIDs.begin();
		trianglePtr!=triangleIDs.end();
		trianglePtr++,nthTriangle++)
	{
		if (inNthBin[nthTriangle]<=bestScheme)
		{
			leftTriangleIDs.push_back(*trianglePtr);
		}
		else
		{
			rightTriangleIDs.push_back(*trianglePtr);
		}
	}
/*	printf("left triangle: %d , right triangle: %d",
		leftTriangleIDs.size(),rightTriangleIDs.size());*/

	delete [] bins;
	delete [] inNthBin;
	delete [] SL;
	delete [] SR;

	bins = NULL;
	inNthBin = NULL;
	SL = NULL;
	SR = NULL;
	//至此，左右节点的三角形数组，对应的包围盒已经建好
	//故创建子树
//	cout<<"clear temp completed."<<endl;
	if(leftTriangleIDs.size()==0 || rightTriangleIDs.size()==0)
		isIll=true;

	buildTreeKernel(leftTriangleIDs,leftBox,depth+1,isIll);

	nodes[currNodeID].farNode = nodes.size();

	buildTreeKernel(rightTriangleIDs,rightBox,depth+1,isIll);
}

void WSimpleBVH::drawTreeIteractively()
{
	glColor3f(1.0f,0.0f,0.0f);
	for(vector<WBVHNode>::iterator pNode = nodes.begin();
		pNode != nodes.end(); pNode++ )
	{
		if((*pNode).type == WBVHNode::BVHN_LEAF)
			continue;
		WBoundingBox box;
		box.pMin = WVector3((*pNode).box[0],
			(*pNode).box[1],
			(*pNode).box[2]);
		box.pMax = WVector3((*pNode).box[3],
			(*pNode).box[4],
			(*pNode).box[5]);
		box.draw();
	}
}

void WSimpleBVH::clearTree()
{
	nodes.clear();
	triangles = NULL;
	delete [] compressedNodes;
	compressedNodes = NULL;
}

void WSimpleBVH::drawTreeRecursivelyKernel( vector<WBVHNode>&nodes, unsigned int ithNode )
{
	if(nodes[ithNode].type==WBVHNode::BVHN_LEAF)
		return;
	WBoundingBox box;
	box.pMin = WVector3(
		nodes[ithNode].box[0],
		nodes[ithNode].box[1],
		nodes[ithNode].box[2]);
	box.pMax = WVector3(
		nodes[ithNode].box[3],
		nodes[ithNode].box[4],
		nodes[ithNode].box[5]);
	box.draw();
	drawTreeRecursivelyKernel(nodes,ithNode+1);
	drawTreeRecursivelyKernel(nodes,nodes[ithNode].farNode);

}

void WSimpleBVH::drawTree( unsigned int nthBox/*=0*/,float R /*= 0.7*/, float G/*= 0.7*/, float B /*= 0.7*/ )
{
	glColor3f(R, G, B);
	this->drawTreeRecursivelyKernel(nodes,0);
}

#ifdef STACKLESS_TRAVERSAL
bool WSimpleBVH::intersect( WRay&r,WDifferentialGeometry&DG, int* endNode, int beginNode /*= 0*/ )
{
	++numIntersect;
	WTriangle* currTriID = NULL;
	float currT = FLT_MAX;
	int currNodeID = 0;
	float boxMin = FLT_MAX;
	WBoundingBox box;
	bool isIntersect;
	float invDx = 1.0f / r.direction.x;
	float invDy = 1.0f / r.direction.y;
	float invDz = 1.0f / r.direction.z;
	while(currNodeID != -1)
	{
		float boxMax;
		//若为内部节点,根据包围盒与光线相交情况决定
		if(isInterior(compressedNodes[currNodeID]))
		{
			++numTraverseInterior_I;
			getBBoxFromCompressedNode(
				box,compressedNodes[currNodeID]);
			//isIntersect = box.isIntersect(r, boxMin, temp);
			
			float t0,t1;
			t0 = (box.pMin.x - r.point.x) * invDx;
			t1 = (box.pMax.x - r.point.x) * invDx;
			boxMin = min(t0,t1);
			boxMax = max(t0,t1);
			t0 = (box.pMin.y - r.point.y) * invDy;
			t1 = (box.pMax.y - r.point.y) * invDy;
			boxMin = max(boxMin,min(t0,t1));
			boxMax = min(boxMax,max(t0,t1));		
			t0 = (box.pMin.z - r.point.z) * invDz;
			t1 = (box.pMax.z - r.point.z) * invDz;
			boxMin = max(boxMin,min(t0,t1));
			boxMax = min(boxMax,max(t0,t1));
			isIntersect = !((boxMin > boxMax) | (r.tMin > boxMax) | (r.tMax < boxMin));

			if(currT > boxMin && isIntersect)
			{
				currNodeID = 
					getOnHitID(compressedNodes[currNodeID]);
			}
			else
			{
				currNodeID = 
					compressedNodes[currNodeID].onMiss;
			}
		}
		//若为叶子节点，对三角形求交
		else
		{
			++numTraverseLeaf_I;
			int nTris = getNTris(compressedNodes[currNodeID]);
			for(int i=0; i<nTris; i++)
			{
				++numTriangleIsect_I;
				WTriangle* tri = compressedNodes[currNodeID].tris[i];
				float testT = tri->intersectTest(r);
				if(testT < currT)
				{
					currT = testT;
					currTriID = 
						compressedNodes[currNodeID].tris[i];
				}				
			}
			currNodeID = compressedNodes[currNodeID].next;
		}
	}
	if(currTriID)
	{
		currTriID->intersect(r, DG);
		return true;
	}
	return false;
}
#else
//利用未压缩的BVH节点求交
bool WSimpleBVH::intersect( WRay&r,WDifferentialGeometry&DG, int beginNode = 0 )
{
	vector<int>nodeIDStack;//记录节点的序号
	vector<float>nodeTStack;//记录光线与节点包围盒求交所得的t
	nodeIDStack.reserve(2*nodeMaxDepth+1);
	nodeTStack.reserve(2*nodeMaxDepth+1);//预留空间

	int currTriID = -1;				//记录当前最先相交的三角形索引
	float currT = FLT_MAX;			//记录光线对应的t值
	int currNodeID = 0;				//当前节点

	nodeIDStack.push_back(0);
	nodeTStack.push_back(0.0f);
	
	WBoundingBox box;
	WBVHNode currNode;				  //当前节点
	WBVHNode childNode[2];			  //内部子节点
	float childT[2];				  //与内部子节点包围盒相交的t
	bool isChildIntersect[2];	   	  //是否与内部子节点包围盒相交
	unsigned int childID[2];		  //子节点索引

	while(nodeIDStack.size()!=0)
	{
		if(nodeTStack.back()>currT)
		{
			//说明没必要对其进一步求交
			nodeIDStack.pop_back();
			nodeTStack.pop_back();
			continue;
		}
		//获得顶层节点，并出栈
		currNodeID = nodeIDStack.back();
		currNode = nodes[currNodeID];
		nodeIDStack.pop_back();
		nodeTStack.pop_back();

		//获得左右子节点
		childID[0] = currNodeID+1;
		childID[1] = currNode.farNode;

		childNode[0] = nodes[childID[0]];
		childNode[1] = nodes[childID[1]];

		isChildIntersect[0] = isChildIntersect[1] = false;

		for(unsigned int i=0;i<2;i++)
		{
			//先检测子节点有没有叶节点
			if(childNode[i].type == WBVHNode::BVHN_LEAF)
			{
				for(unsigned int nthTri = 0;
					nthTri<childNode[i].nTriangles;
					nthTri++)
				{
					WTriangle tri = getTriangle(
						childNode[i].tris[nthTri]);
					float tempT = tri.intersectTest(r);
					//发现更近的交点
					if(tempT < currT)
					{
						currT = tempT;
						currTriID = childNode[i].tris[nthTri];
					}
				}
			}
			//再检测内部节点的情况
			else
			{
				float temp;
				getBBoxFromInterior(box, childNode[i]);
				isChildIntersect[i] = 
					box.isIntersect(r,childT[i],temp);
				//如果相交，把交点值和索引值入栈
				if(isChildIntersect[i])
				{
					nodeIDStack.push_back(childID[i]);
					nodeTStack.push_back(childT[i]);
				}
			}
		}
		//当两个节点都进栈的时候，相交t值较小的一个先进栈
		//这段代码没什么用
		/*
		unsigned int stackSize = nodeIDStack.size();
		if(isChildIntersect[0] && isChildIntersect[1] &&
			childT[0] < childT[1])
		{
			nodeIDStack[stackSize-2] = childID[1];
			nodeIDStack[stackSize-1] = childID[0];

			nodeTStack[stackSize-2] = childT[1];
			nodeTStack[stackSize-1] = childT[0];
		}*/


	}
	if(currTriID != -1)
	{
		WTriangle res = getTriangle(currTriID);
		res.intersect(r, DG);
		return true;
	}
	return false;
}
#endif

#ifdef STACKLESS_TRAVERSAL
bool WSimpleBVH::isIntersect( WRay&r, int beginNode)
{
	++numIntersectTest;
	int currTriID = -1;
	float currT = FLT_MAX;
	int currNodeID = 0;
	float boxMin = FLT_MAX;
	WBoundingBox box;
	bool isIntersect;
	float invDx = 1.0f / r.direction.x;
	float invDy = 1.0f / r.direction.y;
	float invDz = 1.0f / r.direction.z;
	while(currNodeID != -1)
	{
		float boxMax;
		//若为内部节点,根据包围盒与光线相交情况决定
		if(isInterior(compressedNodes[currNodeID]))
		{
			++numTraverseInterior_IT;
			getBBoxFromCompressedNode(
				box,compressedNodes[currNodeID]);
			//isIntersect = box.isIntersect(r, boxMin, temp);
			
			float t0,t1;
			t0 = (box.pMin.x - r.point.x) * invDx;
			t1 = (box.pMax.x - r.point.x) * invDx;
			boxMin = min(t0,t1);
			boxMax = max(t0,t1);
			t0 = (box.pMin.y - r.point.y) * invDy;
			t1 = (box.pMax.y - r.point.y) * invDy;
			boxMin = max(boxMin,min(t0,t1));
			boxMax = min(boxMax,max(t0,t1));		
			t0 = (box.pMin.z - r.point.z) * invDz;
			t1 = (box.pMax.z - r.point.z) * invDz;
			boxMin = max(boxMin,min(t0,t1));
			boxMax = min(boxMax,max(t0,t1));
			isIntersect = ((boxMin < boxMax) && (r.tMin < boxMax) && (r.tMax > boxMin));

			if(currT > boxMin && isIntersect)
			{
				currNodeID = 
					getOnHitID(compressedNodes[currNodeID]);
			}
			else
			{
				currNodeID = 
					compressedNodes[currNodeID].onMiss;
			}
		}
		//若为叶子节点，对三角形求交
		else
		{
			++numTraverseLeaf_IT;
			int nTris = getNTris(compressedNodes[currNodeID]);
			for(int i=0; i<nTris; i++)
			{
				++numTriangleIsect_IT;
				WTriangle* tri = compressedNodes[currNodeID].tris[i];
				float testT = tri->intersectTest(r);
				if(testT < currT)
				{
					return true;
				}
			}
			currNodeID = compressedNodes[currNodeID].next;
		}
	}
	return false;
}
#else
bool WSimpleBVH::isIntersect( WRay&r, int beginNode = 0 )
{
	vector<int>nodeIDStack;//记录节点的序号
	vector<float>nodeTStack;//记录光线与节点包围盒求交所得的t
	nodeIDStack.reserve(2*nodeMaxDepth+1);
	nodeTStack.reserve(2*nodeMaxDepth+1);//预留空间

	int currTriID = -1;				//记录当前最先相交的三角形索引
	float currT = r.tMax;			//记录光线对应的t值
	int currNodeID = 0;				//当前节点

	nodeIDStack.push_back(0);
	nodeTStack.push_back(0.0f);

	WBoundingBox box;
	WBVHNode currNode;				  //当前节点
	WBVHNode childNode[2];			  //内部子节点
	float childT[2];				  //与内部子节点包围盒相交的t
	bool isChildIntersect[2];	   	  //是否与内部子节点包围盒相交
	unsigned int childID[2];		  //子节点索引

	while(nodeIDStack.size()!=0)
	{
		if(nodeTStack.back()>currT)
		{
			//说明没必要对其进一步求交
			nodeIDStack.pop_back();
			nodeTStack.pop_back();
			continue;
		}
		//获得顶层节点，并出栈
		currNodeID = nodeIDStack.back();
		currNode = nodes[currNodeID];
		nodeIDStack.pop_back();
		nodeTStack.pop_back();

		//获得左右子节点
		childID[0] = currNodeID+1;
		childID[1] = currNode.farNode;

		childNode[0] = nodes[childID[0]];
		childNode[1] = nodes[childID[1]];

		isChildIntersect[0] = isChildIntersect[1] = false;

		for(unsigned int i=0;i<2;i++)
		{
			//先检测子节点有没有叶节点
			if(childNode[i].type == WBVHNode::BVHN_LEAF)
			{
				for(unsigned int nthTri = 0;
					nthTri<childNode[i].nTriangles;
					nthTri++)
				{
					WTriangle tri = getTriangle(
						childNode[i].tris[nthTri]);
					float tempT = tri.intersectTest(r);
					//发现更近的交点,立即返回
					if(tempT < currT)
					{
						return true;
					}
				}
			}
			//再检测内部节点的情况
			else
			{
				float temp;
				getBBoxFromInterior(box, childNode[i]);
				isChildIntersect[i] = 
					box.isIntersect(r,childT[i],temp);
				//如果相交，把交点值和索引值入栈
				if(isChildIntersect[i]&&childT[i]<currT)
				{
					nodeIDStack.push_back(childID[i]);
					nodeTStack.push_back(childT[i]);
				}
			}
		}
		//当两个节点都进栈的时候，相交t值较小的一个先进栈
		//似乎这段代码没什么用
		/*
		unsigned int stackSize = nodeIDStack.size();
		if(isChildIntersect[0] && isChildIntersect[1] &&
			childT[0] < childT[1])
		{
			nodeIDStack[stackSize-2] = childID[1];
			nodeIDStack[stackSize-1] = childID[0];

			nodeTStack[stackSize-2] = childT[1];
			nodeTStack[stackSize-1] = childT[0];
		}*/


	}
	return false;
}


#endif
void WSimpleBVH::buildEnhancedTree()
{
	//申请空间
	compressedNodes = new WBVHCompressedNode[nodes.size()];
	//stack A 是用来遍历树的堆栈
	//stack B 是用来给onMiss赋值的堆栈
	vector<int> stackA;
	vector<int> stackB;

	int p, r;	//p记录当前节点索引，r记录当前节点右子节点索引
				//p = -1时表示为空
	p = r = 0; r = nodes[0].farNode;
	unsigned int stackTop = 0;
	while(p != -1 || stackA.size() != 0)
	{
		if(p != -1)
		{
			stackA.push_back(p);
			//复制包围盒或者三角形信息
			compressedNodes[p].box[0] = nodes[p].box[0];
			compressedNodes[p].box[1] = nodes[p].box[1];
			compressedNodes[p].box[2] = nodes[p].box[2];
			compressedNodes[p].box[3] = nodes[p].box[3];
			compressedNodes[p].box[4] = nodes[p].box[4];
			compressedNodes[p].box[5] = nodes[p].box[5];
			if(nodes[p].type != WBVHNode::BVHN_LEAF)
			{	//设置内部节点的type值和onHit值
				setInterior(compressedNodes[p],p+1);
				p++;

			}
			else
			{
				/*
				compressedNodes[p].tris[0] = nodes[p].tris[0];
				compressedNodes[p].tris[1] = nodes[p].tris[1];
				compressedNodes[p].tris[2] = nodes[p].tris[2];
				compressedNodes[p].tris[3] = nodes[p].tris[3];
				compressedNodes[p].tris[4] = nodes[p].tris[4];
				compressedNodes[p].tris[5] = nodes[p].tris[5];
				*/
				//设置叶节点的type值和next值
				setLeaf(compressedNodes[p],
					nodes[p].nTriangles);
				compressedNodes[p].next = p + 1;
				p = -1;
			}
		}
		else
		{
			stackTop = stackA.back();
			if(nodes[stackTop].type != WBVHNode::BVHN_LEAF
				&& nodes[stackTop].farNode != r)
			{	//设置内部节点的onMiss值
				p = nodes[stackTop].farNode;
				for(unsigned int i=0; i<stackB.size(); i++)
				{
					compressedNodes[stackB[i]].onMiss =
						nodes[stackTop].farNode;
				}
				stackB.clear();//清空堆栈
			}
			else
			{
				r = stackA.back();
				stackA.pop_back();
				stackB.push_back(r);
				p = -1;
			}
		}
	}
	for(unsigned int i=0; i<stackB.size(); i++)
	{
		compressedNodes[stackB[i]].onMiss =	-1;
	}
	stackB.clear();
	//设置最后一个叶节点的next值
	compressedNodes[nodes.size()-1].next = -1;
}

void WSimpleBVH::displayNodeInfo()
{
	vector<WBVHNode>::iterator p;
	unsigned int i = 0;
	printf("\n\noriginal nodes\n");
	printf("****************\n");
	return;
	WBoundingBox box;
	for(p = nodes.begin(); p !=nodes.end(); p++,i++)
	{

		if(p->type == WBVHNode::BVHN_LEAF)
		{
			printf("%d th : leaf\n",i);
			printf("%d tris\n",p->nTriangles);
			for(unsigned int j = 0; j<p->nTriangles; j++)
			{
				printf("%d th tri: %d\n",j,p->tris[j]);
			}
		}
		else
		{
			printf("%d th : interior\n",i);
			printf("far: %d\n",p->farNode);
			getBBoxFromInterior(box,*p);
			box.displayCoords();
		}
		printf("****************\n");
	}
}

void WSimpleBVH::displayEnhancedNodeInfo()
{
	printf("\n\ncompressed nodes\n");
	printf("****************\n");
	WBoundingBox box;
	for(unsigned int i = 0; i<nodes.size(); i++)
	{
		//显示内部节点信息
		if(isInterior(compressedNodes[i]))
		{
			printf("%d th : interior\n",i);
			printf("onHit: %d\n",
				getOnHitID(compressedNodes[i]) );
			printf("onMiss: %d\n",
				compressedNodes[i].onMiss);
			getBBoxFromCompressedNode(box,compressedNodes[i]);
			box.displayCoords();
		}
		//显示叶节点信息
		else
		{
			printf("%d th : leaf\n",i);
			printf("next: %d \n",compressedNodes[i].next);
			printf("%d tris\n",getNTris(compressedNodes[i]));
			for(int j = 0; j<getNTris(compressedNodes[i]); j++)
			{
				printf("%d th tri: %d\n",
					j,compressedNodes[i].tris[j]);
			}
		}
		printf("****************\n");
	}
}