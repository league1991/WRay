#include "StdAfx.h"
#include "KDTree.h"

WKDTree::WKDTree(void)
{
	nMailBoxes=0;
	nAllocatedNodes=128;
	nUsedNodes=0;
	currRayID=0;
	maxDepth=15;
	maxSubPsPerNode=3;
	traversalCost=10.0f;
	isectCost=1.0f;
	nodes=NULL;
	nodeBoxes=NULL;
	mailBoxes=NULL;
}

WKDTree::~WKDTree(void)
{
	clearTree();
}
void WKDTree::setNodeAttr(
				 unsigned int imaxDepth,
				 unsigned int imaxSubPsPerNode,
				 float iisectCost,
				 float itraversalCost
				 )
{
	if(imaxDepth>1)maxDepth=imaxDepth;
	if(imaxSubPsPerNode>1)maxSubPsPerNode=imaxSubPsPerNode;
	if(iisectCost>0)isectCost=iisectCost;
	if(itraversalCost>0)traversalCost=itraversalCost;
	return;
}

void WKDTree::clearTree()
{
	for(unsigned int nthNode=0;nthNode<nUsedNodes;nthNode++)
	{
		if(nodes[nthNode].type==WKDNode::KDN_LEAF)
		{
			//删除每个节点的mailbox索引数组
			delete[]nodes[nthNode].mailBoxIndices;
		}
	}
	delete []nodes;
	delete []mailBoxes;
	delete []nodeBoxes;
	nodes=NULL;
	mailBoxes=NULL;
	nodeBoxes=NULL;
	nUsedNodes=0;
	currRayID=0;
}
void WKDTree::cleanMailBoxes()
{
	WMailBox*p=mailBoxes;
	for(unsigned int i=0;i<nMailBoxes;i++)
	{
		p->rayID=0;
		p++;
	}
}
void WKDTree::buildMailBoxes(WScene&scene)
{
	nMailBoxes=scene.getSubPrimNum();
	mailBoxes=new WMailBox[nMailBoxes];
	WPrimitive*prims;
	unsigned int nPrims;
	WSubPrimitive*subPs;
	unsigned int nSubPs;
	scene.getPrimitives(prims,nPrims);
	unsigned int currBoxID=0;//当前mailbox的下标
	for(unsigned int i=0;i<nPrims;i++)
	{
		prims[i].getSubPrimitives(subPs,nSubPs);
		for(unsigned int j=0;j<nSubPs;j++)
		{
			mailBoxes[currBoxID].pSubP=subPs;
			subPs++;
			mailBoxes[currBoxID].rayID=0;
			currBoxID++;
		}
	}
}
void WKDTree::drawMailBoxes(	
						   float R,
						   float G,
						   float B)
{
	glColor3f(R,G,B);
	for(unsigned int i=0;i<nMailBoxes;i++)
	{
		mailBoxes[i].pSubP->box.draw();//画出包围盒
	}
	//cout<<nMailBoxes;
}
void WKDTree::makeLeaf(unsigned int nMBoxes,
					  const vector<int>&mBoxIndices,
					  vector<WKDNode>&vNodes)
{
	WKDNode node;
	node.nMailBoxes=nMBoxes;
	node.mailBoxIndices=new int[nMBoxes];
	for(unsigned int i=0;i<nMBoxes;i++)
		node.mailBoxIndices[i]=mBoxIndices[i];
	node.type=WKDNode::KDN_LEAF;
	vNodes.push_back(node);
	/*		cout<<"return in depth"<<depth<<endl;*/
	return;
}
void WKDTree::updateProcess(const float &depth)
{
	float newProcess=process+pow(2.0f,-1.0f*depth);
	if(int(process*20)!=int(newProcess*20))
		cout<<"tree building process:"<<int(newProcess*100)<<'%'<<endl;
	process=newProcess;
	return ;
}
void WKDTree::buildTreeCore(
						   unsigned int depth,
						   const vector<int>&mBoxIndices,
						   const WBoundingBox&bBox,
						   vector<WKDNode>&vNodes,
						   vector<WBoundingBox>&vBoxes)
{
	unsigned int nMBoxes;
	WKDNode*currNode;
	WBoundingBox*currBBox;
	nMBoxes=mBoxIndices.size();

	//预留空间
	if(nUsedNodes==nAllocatedNodes)
	{	
		nAllocatedNodes*=2;
		vNodes.reserve(nAllocatedNodes);
		vBoxes.reserve(nAllocatedNodes);

	}
	nUsedNodes++;
	//获得当前的节点和对应的BoundingBox
	currNode=new WKDNode;
	currBBox=new WBoundingBox;
	*currBBox=bBox;
	vBoxes.push_back(*currBBox);

// 	cout<<"\n##############depth begin##############"<<depth<<endl;
//  	cout<<"nUsedNodes"<<nUsedNodes<<endl;
//  	cout<<"\nmBoxIndices"<<endl;
// 	for(unsigned int i=0;i<nMBoxes;i++)
// 		cout<<mBoxIndices[i]<<ends;
// 	cout<<endl;

	//符合条件就新建叶节点
	if(depth==maxDepth||nMBoxes<=maxSubPsPerNode||!nMBoxes)
	{
		//cout<<"depth"<<depth<<endl;
		makeLeaf(nMBoxes,mBoxIndices,vNodes);
		updateProcess(depth);
/*		cout<<"return in depth"<<depth<<endl;*/
		return;
	}
	//找出坐标范围最大的轴向分割
	int splitAxis=currBBox->maxAxis();
// 	cout<<"\nsplit Axis"<<splitAxis<<endl;
// 
// 	cout<<"build BoundingEdge begin"<<endl;
	vector<BoundingEdge>edges;	
	vector<BoundingEdge>::iterator pEdge;
	//预留空间，boundingEdge的数量是BoundingBox的两倍
//	cout<<"nMboxes"<<nMBoxes<<endl;
// 	edges.reserve(nMBoxes*2);
	//edges.clear();
 	edges.resize(nMBoxes*2);
	//初始化boundingEdge
/*	cout<<"resized"<<endl;*/
	pEdge=edges.begin();
/*	cout<<"beginloop"<<endl;*/
	for(unsigned int i=0;
		i<nMBoxes;i++,pEdge++)
	{
		int ithMailBox=mBoxIndices[i];
/*		cout<<ithMailBox<<ends;*/
//		BoundingEdge tempEdge;
		switch (splitAxis)
		{
		case 0:
			pEdge->t=mailBoxes[ithMailBox].pSubP->box.pMin.x;
			pEdge->type=BoundingEdge::BE_START;
			pEdge->mailBoxNum=ithMailBox;

			pEdge++;
			pEdge->t=mailBoxes[ithMailBox].pSubP->box.pMax.x;
			pEdge->type=BoundingEdge::BE_END;
			pEdge->mailBoxNum=ithMailBox;

// 			tempEdge.t=mailBoxes[ithMailBox].pSubP->box.pMin.x;
// 			tempEdge.type=BoundingEdge::BE_START;
// 			tempEdge.mailBoxNum=i;
// 			
// 			edges.push_back(tempEdge);
// 			tempEdge.t=mailBoxes[ithMailBox].pSubP->box.pMax.x;
// 			tempEdge.type=BoundingEdge::BE_END;
// 			tempEdge.mailBoxNum=i;
// 			edges.push_back(tempEdge);
			break;
		case 1:
			pEdge->t=mailBoxes[ithMailBox].pSubP->box.pMin.y;
			pEdge->type=BoundingEdge::BE_START;
			pEdge->mailBoxNum=ithMailBox;

			pEdge++;
			pEdge->t=mailBoxes[ithMailBox].pSubP->box.pMax.y;
			pEdge->type=BoundingEdge::BE_END;
			pEdge->mailBoxNum=ithMailBox;

// 			tempEdge.t=mailBoxes[ithMailBox].pSubP->box.pMin.y;
// 			tempEdge.type=BoundingEdge::BE_START;
// 			tempEdge.mailBoxNum=i;
// 			edges.push_back(tempEdge);
// 			tempEdge.t=mailBoxes[ithMailBox].pSubP->box.pMax.y;
// 			tempEdge.type=BoundingEdge::BE_END;
// 			tempEdge.mailBoxNum=i;
// 			edges.push_back(tempEdge);
			break;
		case 2:
			pEdge->t=mailBoxes[ithMailBox].pSubP->box.pMin.z;
			pEdge->type=BoundingEdge::BE_START;
			pEdge->mailBoxNum=ithMailBox;

			pEdge++;
			pEdge->t=mailBoxes[ithMailBox].pSubP->box.pMax.z;
			pEdge->type=BoundingEdge::BE_END;
			pEdge->mailBoxNum=ithMailBox;

// 			tempEdge.t=mailBoxes[ithMailBox].pSubP->box.pMin.z;
// 			tempEdge.type=BoundingEdge::BE_START;
// 			tempEdge.mailBoxNum=i;
// 			edges.push_back(tempEdge);
// 			tempEdge.t=mailBoxes[ithMailBox].pSubP->box.pMax.z;
// 			tempEdge.type=BoundingEdge::BE_END;
// 			tempEdge.mailBoxNum=i;
// 			edges.push_back(tempEdge);
		}

	}
/*	cout<<"build BoundingEdge end"<<endl;*/
	//根据坐标大小对BoundingEdge进行排序
	stable_sort(edges.begin(),edges.end());

//   	cout<<"sorted"<<endl;
//  	for(pEdge=edges.begin();pEdge!=edges.end();pEdge++)
//  		cout<<pEdge->mailBoxNum<<ends;
//  	cout<<endl;

	//找出bBox对应轴向的坐标范围
	float bBoxMin,bBoxMax,invBBoxDelta;
	switch (splitAxis)
	{
	case 0:
		bBoxMin=bBox.pMin.x;
		bBoxMax=bBox.pMax.x;
		break;
	case 1:
		bBoxMin=bBox.pMin.y;
		bBoxMax=bBox.pMax.y;
		break;
	case 2:
		bBoxMin=bBox.pMin.z;
		bBoxMax=bBox.pMax.z;
	}
	invBBoxDelta=1.0f/(bBoxMax-bBoxMin);

/*	cout<<"compute best position begin"<<endl;*/

	//选择最佳划分
	//以下是两个度量划分优劣的参数
	float oldCost=isectCost*nMBoxes;
	float bestCost=M_INF_BIG;
	float cost;
	float pisect;//相交概率
	float bestT=bBoxMin;
	unsigned int nBelow=0;
	unsigned int nAbove=nMBoxes;
	unsigned int bestBelow=0,bestAbove=nAbove;
//	unsigned int bestOffset=0;
	float isEmpty;
//	int nthEdge;//记录最佳划分对应的BoundingEdge
/*	cout<<"edges"<<endl;*/
	for(pEdge=edges.begin()/*,nthEdge=0*/;
		pEdge!=edges.end();pEdge++/*,nthEdge++*/)
	{
		//cout<<nthEdge<<endl;
		float edgeT=pEdge->t;
/*		cout<<pEdge->mailBoxNum<<ends;*/
		if(edgeT>bBoxMin+1e-5f
			&& edgeT<bBoxMax-1e-5f)//确保不出现体积为0的节点
		{ 

			pisect=(edgeT-bBoxMin)*invBBoxDelta;
			if(nBelow==0)//第一条边
				isEmpty=pisect;
			else if(nAbove==1)//最后一条边
				isEmpty=1-pisect;
			else
				isEmpty=0;
			//cout<<isEmpty<<ends;			
			cost=traversalCost+
				isectCost*(1-isEmpty)*
				(pisect*nBelow+(1-pisect)*nAbove);
// 			cout<<" pisect "<<pisect
// 				<<" cost "<<cost
// 				<<" bestCost "<<bestCost
// 				<<" bestBelow "<<bestBelow
// 				<<" bestAbove "<<bestAbove<<endl;
			if(cost<bestCost)
			{
				bestCost=cost;
//				bestOffset=nthEdge;
				bestBelow=nBelow;
				bestAbove=nAbove;
				bestT=edgeT;
			}
		}
		if(pEdge->type==BoundingEdge::BE_START)
		{
			nBelow++;
		}
		else //也就是BoundingEdge::BE_END
		{
			nAbove--;
		}
	}
// 	cout<<"bestT"<<bestT<<endl;
// 	cout<<"best below"<<bestBelow<<endl;
// 	cout<<"best above"<<bestAbove<<endl;
// 	cout<<"bestCost"<<bestCost<<"oldCost"<<oldCost<<endl;
	//如果没有合适的分割，就在当前位置新建叶节点
	if(bestCost>oldCost*2)
	{
		makeLeaf(nMBoxes,mBoxIndices,vNodes);
		updateProcess(depth);
		return;
	}


// 	cout<<endl;
// 	cout<<"split mailboxes begin"<<endl;
// 	for(pEdge=edges.begin();pEdge!=edges.end();pEdge++)
// 		cout<<pEdge->mailBoxNum<<ends;

	//把两侧的mailBox分别加入对应的mailBox数组
/*
	int*belowMBoxIndices,*aboveMBoxIndices;
	if(bestBelow!=0)
		belowMBoxIndices=new int[bestBelow];
	else
		belowMBoxIndices=NULL;
	if(bestAbove!=0)
		aboveMBoxIndices=new int[bestAbove];
	else
		aboveMBoxIndices=NULL;
	unsigned int belowIndex=0,aboveIndex=0;
	for(pEdge=edges.begin();
		pEdge!=edges.end();pEdge++)
	{
		if(pEdge->type==BoundingEdge::BE_START
			&&pEdge->t<bestT)
		{
			belowMBoxIndices[belowIndex]=pEdge->mailBoxNum;
			belowIndex++;
		}
		if(pEdge->type==BoundingEdge::BE_END
			&&pEdge->t>=bestT)
		{
			aboveMBoxIndices[aboveIndex]=pEdge->mailBoxNum;
			aboveIndex++;
		}

	}*/

	vector<int>belowMBIndices;
	vector<int>aboveMBIndices;
	vector<int>::iterator pi;
	for(pEdge=edges.begin();
		pEdge!=edges.end();pEdge++)
	{
/*		cout<<pEdge->mailBoxNum<<endl;*/
		if(pEdge->type==BoundingEdge::BE_START
			&&pEdge->t<bestT)
		{
// 			cout<<"below"<<ends;
// 			cout<<pEdge->mailBoxNum<<endl;
			belowMBIndices.push_back(pEdge->mailBoxNum);
		}
		if(pEdge->type==BoundingEdge::BE_END
			&&pEdge->t>=bestT)
		{
// 			cout<<"above"<<ends;
// 			cout<<pEdge->mailBoxNum<<endl;
			aboveMBIndices.push_back(pEdge->mailBoxNum);
		}

	}
// 	cout<<"below"<<endl;
// 	for(pi=belowMBIndices.begin();
// 		pi!=belowMBIndices.end();pi++)
// 		cout<<*pi<<ends;
// 	cout<<endl;
// 	cout<<"above"<<endl;
// 	for(pi=aboveMBIndices.begin();
// 		pi!=aboveMBIndices.end();pi++)
// 		cout<<*pi<<ends;
// 	cout<<endl;




	/*
	switch (splitAxis)
	{
	case 0:	
		for(unsigned int i=0;i<nMBoxes;i++)
		{
			if(mailBoxes[mBoxIndices[i]].pSubP->box.pMin.x<bestT)
			{
				belowMBoxIndices[belowIndex]=mBoxIndices[i];
				belowIndex++;
			}
			if(mailBoxes[mBoxIndices[i]].pSubP->box.pMax.x>=bestT)
			{
				//cout<<"above"<<endl;
				aboveMBoxIndices[aboveIndex]=mBoxIndices[i];
				aboveIndex++;
			}
		}
			break;
	case 1:	
		for(unsigned int i=0;i<nMBoxes;i++)
		{
			if(mailBoxes[mBoxIndices[i]].pSubP->box.pMin.y<bestT)
			{
				belowMBoxIndices[belowIndex]=mBoxIndices[i];
				belowIndex++;
			}
			if(mailBoxes[mBoxIndices[i]].pSubP->box.pMax.y>=bestT)
			{
				aboveMBoxIndices[aboveIndex]=mBoxIndices[i];
				aboveIndex++;
			}
		}
		break;
	case 2:	
		for(unsigned int i=0;i<nMBoxes;i++)
		{
			if(mailBoxes[mBoxIndices[i]].pSubP->box.pMin.z<bestT)
			{
				belowMBoxIndices[belowIndex]=mBoxIndices[i];
				belowIndex++;
			}
			if(mailBoxes[mBoxIndices[i]].pSubP->box.pMax.z>=bestT)
			{
				aboveMBoxIndices[aboveIndex]=mBoxIndices[i];
				aboveIndex++;
			}
		}
	}*/
// 	cout<<"\nbelowIndices"<<endl;
// 	for(int i=0;i<bestBelow;i++)
// 		cout<<belowMBoxIndices[i]<<ends;
// 	cout<<"\naboveIndices"<<endl;
// 	for(int i=0;i<bestAbove;i++)
// 		cout<<aboveMBoxIndices[i]<<ends;


/*	cout<<"compute two boundingboxes begin"<<endl;*/
	//计算分割平面两侧的BoundingBox
	WBoundingBox belowBox,aboveBox;
	belowBox=aboveBox=bBox;
/*	cout<<"bestT"<<bestT<<endl;*/
	switch(splitAxis)
	{
	case 0:
		belowBox.pMax.x=aboveBox.pMin.x=bestT;
		currNode->type=WKDNode::KDN_XSPLIT;
		break;
	case 1:
		belowBox.pMax.y=aboveBox.pMin.y=bestT;					currNode->type=WKDNode::KDN_YSPLIT;
		break;
	case 2:
		belowBox.pMax.z=aboveBox.pMin.z=bestT;
		currNode->type=WKDNode::KDN_ZSPLIT;
	}
	unsigned int nextDepth=depth+1;

// 	bBox.displayCoords();
//	belowBox.displayCoords();
// 	aboveBox.displayCoords();
	//构建below子节点
//	cout<<"build subTree 1 begin"<<endl;
	currNode->splitPos=bestT;
	vNodes.push_back(*currNode);
	unsigned int currIndices=vNodes.size()-1;
	buildTreeCore(nextDepth,belowMBIndices,belowBox,vNodes,vBoxes);

	//currNode->type=(WKDNode::NodeType)splitAxis;
	vNodes[currIndices].aboveChild=nUsedNodes;



/*	cout<<"build subTree 2 begin"<<endl;*/
	//构建above子节点
	buildTreeCore(nextDepth,aboveMBIndices,aboveBox,vNodes,vBoxes);
	delete currNode;
	delete currBBox;
// 	cout<<"delete memory begin"<<endl;
// 
// 	if(belowMBoxIndices)
// 		delete []belowMBoxIndices;
// 	if(aboveMBoxIndices)
// 		delete []aboveMBoxIndices;
// 	belowMBoxIndices=aboveMBoxIndices=NULL;
// 	cout<<"delete memory end"<<endl;
}
void WKDTree::buildTree(WScene&scene)
{
	delete[]mailBoxes;
	delete[]nodeBoxes;
	delete[]nodes;
	WClock timer;
	timer.begin();
	buildMailBoxes(scene);
	sceneBox=scene.getBBox();
 	cout<<"nMailBoxes"<<nMailBoxes<<endl;
// 		int*mBoxIndices=new int[nMailBoxes];
	vector<int>mBIndices;
	vector<WKDNode>vNodes;
	vector<WBoundingBox>vBoxes;
	mBIndices.resize(nMailBoxes);
	vNodes.clear();
	vNodes.reserve(nAllocatedNodes);
	vBoxes.reserve(nAllocatedNodes);
// 	for(unsigned int i=0;i<nMailBoxes;i++)
// 		mBoxIndices[i]=i;
	for(unsigned int i=0;i<nMailBoxes;i++)
		mBIndices[i]=i;
	WBoundingBox bBox=scene.getBBox();
	process=0.0f;
	buildTreeCore(0,mBIndices,bBox,vNodes,vBoxes);
	/*	delete[]mBoxIndices;*/
	this->nodes=new WKDNode[nUsedNodes];
	this->nodeBoxes=new WBoundingBox[nUsedNodes];
	for(unsigned int i=0;i<nUsedNodes;i++)
	{
		nodes[i]=vNodes[i];
		nodeBoxes[i]=vBoxes[i];
	}
	cout<<"nNodes: "<<nUsedNodes<<endl;
/*	delete[]nodeBoxes;
	nodeBoxes=NULL;*/
	vNodes.clear();
	vBoxes.clear();
	timer.end();
	cout<<"build complete.Total time: "<<endl;
	timer.display();
	cout<<"nNodes: "<<nUsedNodes<<endl;
}
void WKDTree::drawTree(unsigned int nthBox,
					  float R,float G,float B)
{
	if(nodeBoxes == NULL)
		return;
	glColor3f(R,G,B);
	//cout<<nUsedNodes<<endl;
	if(nUsedNodes==0)
		return;
/* 	for(unsigned int i=0;i<nUsedNodes;i++)
 		nodeBoxes[i].draw();*/
	if(nodes[nthBox].type==WKDNode::KDN_LEAF)
	{
//		cout<<"leaf"<<endl;
		nodeBoxes[nthBox].draw();
		return;
	}
	else
	{
//		cout<<"above"<<endl;
		nodeBoxes[nthBox].draw();
		unsigned int aboveIndices=
			nodes[nthBox].aboveChild;
		drawTree(nthBox+1,R,G,B);
		drawTree(aboveIndices,R,G,B);
		return;
	}
}
void WKDTree::drawScene(unsigned int nthBox,bool isFill)
{
	if(nUsedNodes==0)
		return;
	if(nodes[nthBox].type==WKDNode::KDN_LEAF)
	{
		int *mBIndices=nodes[nthBox].mailBoxIndices;
		unsigned int nMBs=nodes[nthBox].nMailBoxes;
		for(unsigned int i=0;i<nMBs;i++)
		{
			this->mailBoxes[mBIndices[i]].pSubP->draw(true);
		}
		return;
	}
	else
	{
		unsigned int aboveIndices=nodes[nthBox].aboveChild;
		drawScene(nthBox+1,isFill);
		drawScene(aboveIndices,isFill);
		return;
	}
}
void WKDTree::displayNodeInfo()
{
	for(unsigned int i=0;i<nUsedNodes;i++)
	{
		cout<<i<<' '<<nodes[i].type<<ends;
		if(nodes[i].type!=WKDNode::KDN_LEAF)
			cout<<nodes[i].aboveChild;
		else
			cout<<"nMailBoxes"<<nodes[i].nMailBoxes;
		cout<<endl;
	}
	return;
}

void WKDTree::displayTime()
{
	clock.display();
}
bool WKDTree::intersect(WRay &r, WDifferentialGeometry &DG,
					   int& endNode, int beginNode)
{
//	cout<<"\n\n#################intersection test begin#########"<<endl;

	//记录光线在节点包围盒内部的参数有效范围的变量，
	//这两个变量不影响光线自身的参数范围r.tMin,r.tMax
	//其作用仅限于同tSplit配合确定远近节点，
	//以及是否要对两个子节点都进行测试
	float tMin,tMax;
//	r.normalizeDir();//方向向量单位化

	//sceneBox.displayCoords();

	//如果跟场景不相交，直接返回
	if(!sceneBox.isIntersect(r,tMin,tMax))
		return false;
	if(nUsedNodes==0)//KD树为空
		return false;
	currRayID++;

	WKDNode*pCurrNode=&nodes[0];
	//"近"节点和“远”节点的指针
	WKDNode*pNearNode,*pFarNode;
	WKDNode*pNextNode;//下一次循环要计算的节点
	float rayDComponent;//方向向量在分隔轴向的分量
	float rayPComponent;//起点在分隔轴向的分量
	float tSplit;		//分隔平面的光线t值
	vector<WKDToDo>toDo;

//	clock.begin();
	while(1)
	{
// 		r.tMin=tMin;
// 		r.tMax=tMax;//光线参数范围
//		cout<<"\nminmax"<<tMin<<' '<<tMax<<endl;
		if(pCurrNode->type==WKDNode::KDN_LEAF)
			goto LEAF;
		switch(pCurrNode->type)
		{
		case WKDNode::KDN_XSPLIT:
			rayDComponent=r.direction.x;
			rayPComponent=r.point.x;break;
		case WKDNode::KDN_YSPLIT:
			rayDComponent=r.direction.y;
			rayPComponent=r.point.y;break;
		case WKDNode::KDN_ZSPLIT:
			rayDComponent=r.direction.z;
			rayPComponent=r.point.z;
		}
//		cout<<"pCurrNodeType"<<pCurrNode->type<<endl;

		//前提是当前节点是内部节点
		if(rayDComponent>1e-9f)
		{
			pNearNode=pCurrNode+1;
			pFarNode=&nodes[pCurrNode->aboveChild];
		}
		else if(rayDComponent<-1e-9f)
		{
			pNearNode=&nodes[pCurrNode->aboveChild];
			pFarNode=pCurrNode+1;
		}
		else//光线在分割轴向的方向向量的分量为0
		{
			if(rayPComponent>pCurrNode->splitPos)
			{
				pNextNode=&nodes[pCurrNode->aboveChild];
			}
			else
			{
				pNextNode=pCurrNode+1;
			}
		}
		//对于上两种情况，判断是否要计算两个子节点
		if(rayDComponent>1e-9f||rayDComponent<-1e-9f)
		{
			//计算tSplit
			tSplit=(pCurrNode->splitPos-rayPComponent)/rayDComponent;
			//下面两种情况，
			//可以判定光线跟一个子节点不会相交，
			//故下一步只需要计算一个子节点
			if(tSplit<tMin)
			{
				pNextNode=pFarNode;
			}
			else if(tSplit>tMax)
			{
				pNextNode=pNearNode;
			}
			//这种情况，先计算较近的子节点，然后计算较远的子节点
			else
			{
				pNextNode=pNearNode;
				WKDToDo nextToDo;
				nextToDo.pNode=pFarNode;
				nextToDo.tMin=tSplit;
				nextToDo.tMax=tMax;
				tMax=tSplit;
				toDo.push_back(nextToDo);
	//			cout<<"pushed tSplit="<<tSplit<<endl;
			}
		}
		//cout<<"splitPos"<<pNextNode->splitPos<<endl;
		if(pNextNode->type!=WKDNode::KDN_LEAF)//所选节点为内节点
		{
			pCurrNode=pNextNode;
			continue;
		}
		else//为叶节点
		{
			pCurrNode=pNextNode;
LEAF:
			WTriangle bestTriangle;//记录最近的相交三角形
			float bestT=r.tMax;//最近的光线t值
			float oldTMax=r.tMax;//光线本来的tMax
			int*mBIndices=pCurrNode->mailBoxIndices;//mailbox下标数组
			unsigned int nMBs=
				(unsigned int)pCurrNode->nMailBoxes;//mailBox下标数
//			cout<<"nMailBoxes"<<nMBs<<endl;
			WMailBox*pMB;//当前mailbox指针
			//对所有mailbox求交
			for(unsigned int nthMB=0;nthMB<nMBs;nthMB++)
			{
				pMB=&mailBoxes[mBIndices[nthMB]];
//				glColor3f(0,0.7,0);
				//画出要计算的每个mailbox的包围盒
//				pMB->pSubP->box.draw();
				float t;//相交的t值
				WTriangle tri;//对应的三角形
				
				//以下条件表示这个mailBox包围盒与光线不相交
				//或内部的面与光线不相交
				if(pMB->rayID==currRayID)					
				{
					continue;
				}
				else
				{
					bool isIntersect=false;//记录光线是否与其相交的变量
					float mBTMin,mBTMax;//这两个变量似乎没什么用
					//对于每个mailbox，
					//如果与subP的包围盒相交，进一步计算交点
					if(pMB->pSubP->box.isIntersect(r,mBTMin,mBTMax))
					{
						//如果相交，对mailbox内的三角形求交
						unsigned int beginIndex=pMB->pSubP->beginIndex;
						unsigned int nFaces=pMB->pSubP->nFaces+beginIndex;
//						cout<<beginIndex<<"*****"<<nFaces<<endl;
//						clock.begin();
						for(unsigned int nthFace=beginIndex;
							nthFace<nFaces;
							nthFace++)
						{
							//对每个mailbox内部的每个三角形求交
							pMB->pSubP->pPrimitive->getTriangle(nthFace,tri);
							//cout<<"isSucceed"<<is<<endl;
							t=tri.intersectTest(r);

							//画出每个进行相交测试的三角形
						//	tri.draw(false,true);
							if(t<bestT)//找到更近的交点
							{
								//isIntersect表示
								//光线同mailbox里面的三角形有交点
								isIntersect=true;
								bestTriangle=tri;
								bestT=t;
//								cout<<"update"<<endl;
								//r.tMax=bestT;
								//这一步主要是想
								//减少tri.intersectTest(r)函数运算量
								//现在不用
							}
						} 

										
//						clock.end();
					}
					if(!isIntersect)
						pMB->rayID=currRayID;
					//说明此mailbox内的面与光线不相交	
					//或mailbox的包围盒与光线不相交
				}
			}
//			cout<<setprecision(10)<<"bestT"<<bestT<<endl;
//			cout<<setprecision(10)<<"oldTMax"<<oldTMax<<endl;
			//求交成功,且交点在节点（而不是mailbox）包围盒内
			if(bestT<r.tMax&&bestT<=tMax&&bestT>=tMin)
			{
//				cout<<"**************find intersection************"<<endl;
//				clock.begin();
				bestTriangle.intersect(r,DG);
//				clock.end();
//				DG.position.showCoords();
//				clock.end();
				return true;
			}
			else//求交失败
			{
//				cout<<"**************not find intersection************"<<endl;
				if(toDo.size()==0)//这表示光线与整个KD树都没有交点
				{
//					clock.end();
					return false;
				}
				WKDToDo nextToDo=toDo.back();
				toDo.pop_back();
//				cout<<"pop"<<endl;
				pCurrNode=nextToDo.pNode;
				tMin=nextToDo.tMin;
				tMax=nextToDo.tMax;
//				cout<<"min"<<tMin<<"max"<<tMax<<endl;
			}
		}
	}
}
bool WKDTree::isIntersect(WRay &r, int beginNode)
{
	//	cout<<"\n\n#################intersection test begin#########"<<endl;

	//记录光线在节点包围盒内部的参数有效范围的变量，
	//这两个变量不影响光线自身的参数范围r.tMin,r.tMax
	//其作用仅限于同tSplit配合确定远近节点，
	//以及是否要对两个子节点都进行测试
	float tMin,tMax;
	//r.normalizeDir();//方向向量单位化

	//sceneBox.displayCoords();

	//如果跟场景不相交，直接返回
	if(!sceneBox.isIntersect(r,tMin,tMax))
		return false;
	if(nUsedNodes==0)//KD树为空
		return false;
	currRayID++;

	WKDNode*pCurrNode=&nodes[0];
	//"近"节点和“远”节点的指针
	WKDNode*pNearNode,*pFarNode;
	WKDNode*pNextNode;//下一次循环要计算的节点
	float rayDComponent;//方向向量在分隔轴向的分量
	float rayPComponent;//起点在分隔轴向的分量
	float tSplit;		//分隔平面的光线t值
	vector<WKDToDo>toDo;

	while(1)
	{
		// 		r.tMin=tMin;
		// 		r.tMax=tMax;//光线参数范围
		//		cout<<"\nminmax"<<tMin<<' '<<tMax<<endl;

		if(pCurrNode->type==WKDNode::KDN_LEAF)
			goto LEAF;
		switch(pCurrNode->type)
		{
		case WKDNode::KDN_XSPLIT:
			rayDComponent=r.direction.x;
			rayPComponent=r.point.x;break;
		case WKDNode::KDN_YSPLIT:
			rayDComponent=r.direction.y;
			rayPComponent=r.point.y;break;
		case WKDNode::KDN_ZSPLIT:
			rayDComponent=r.direction.z;
			rayPComponent=r.point.z;
		}
		//		cout<<"pCurrNodeType"<<pCurrNode->type<<endl;

		//前提是当前节点是内部节点
		if(rayDComponent>1e-9f)
		{
			pNearNode=pCurrNode+1;
			pFarNode=&nodes[pCurrNode->aboveChild];
		}
		else if(rayDComponent<-1e-9f)
		{
			pNearNode=&nodes[pCurrNode->aboveChild];
			pFarNode=pCurrNode+1;
		}
		else//光线在分割轴向的方向向量的分量为0
		{
			if(rayPComponent>pCurrNode->splitPos)
			{
				pNextNode=&nodes[pCurrNode->aboveChild];
			}
			else
			{
				pNextNode=pCurrNode+1;
			}
		}
		//对于上两种情况，判断是否要计算两个子节点
		if(rayDComponent>1e-9f||rayDComponent<-1e-9f)
		{
			//计算tSplit
			tSplit=(pCurrNode->splitPos-rayPComponent)/rayDComponent;
			//下面两种情况，
			//可以判定光线跟一个子节点不会相交，
			//故下一步只需要计算一个子节点
			if(tSplit<tMin)
			{
				pNextNode=pFarNode;
			}
			else if(tSplit>tMax)
			{
				pNextNode=pNearNode;
			}
			//这种情况，先计算较近的子节点，然后计算较远的子节点
			else
			{
				pNextNode=pNearNode;
				WKDToDo nextToDo;
				nextToDo.pNode=pFarNode;
				nextToDo.tMin=tSplit;
				nextToDo.tMax=tMax;
				tMax=tSplit;
				toDo.push_back(nextToDo);
				//			cout<<"pushed tSplit="<<tSplit<<endl;
			}
		}
		//cout<<"splitPos"<<pNextNode->splitPos<<endl;
		if(pNextNode->type!=WKDNode::KDN_LEAF)//所选节点为内节点
		{
			pCurrNode=pNextNode;
			continue;
		}
		else//为叶节点
		{
			pCurrNode=pNextNode;
LEAF:
			WTriangle bestTriangle;//记录最近的相交三角形
			float bestT=r.tMax;//最近的光线t值
			float oldTMax=r.tMax;//光线本来的tMax
			int*mBIndices=pCurrNode->mailBoxIndices;//mailbox下标数组
			unsigned int nMBs=
				(unsigned int)pCurrNode->nMailBoxes;//mailBox下标数
			//			cout<<"nMailBoxes"<<nMBs<<endl;
			WMailBox*pMB;//当前mailbox指针
			//对所有mailbox求交
			for(unsigned int nthMB=0;nthMB<nMBs;nthMB++)
			{
				pMB=&mailBoxes[mBIndices[nthMB]];
				//				glColor3f(0,0.7,0);
				//画出要计算的每个mailbox的包围盒
				//				pMB->pSubP->box.draw();
				float t;//相交的t值
				WTriangle tri;//对应的三角形

				//以下条件表示这个mailBox包围盒与光线不相交
				//或内部的面与光线不相交
				if(pMB->rayID==currRayID)					
				{
					continue;
				}
				else
				{
					bool isIntersect=false;//记录光线是否与其相交的变量
					float mBTMin,mBTMax;//这两个变量似乎没什么用
					//对于每个mailbox，
					//如果与subP的包围盒相交，进一步计算交点
					if(pMB->pSubP->box.isIntersect(r,mBTMin,mBTMax))
					{
						//如果相交，对mailbox内的三角形求交
						unsigned int beginIndex=pMB->pSubP->beginIndex;
						unsigned int nFaces=pMB->pSubP->nFaces+beginIndex;
						//cout<<beginIndex<<"*****"<<nFaces<<endl;

						for(unsigned int nthFace=beginIndex;
							nthFace<nFaces;
							nthFace++)
						{
							//对每个mailbox内部的每个三角形求交
							pMB->pSubP->pPrimitive->getTriangle(nthFace,tri);
							//cout<<"isSucceed"<<is<<endl;
							t=tri.intersectTest(r);

							//画出每个进行相交测试的三角形
							//	tri.draw(false,true);
							if(t<r.tMax)//找到更近的交点
							{
								//isIntersect表示
								//光线同mailbox里面的三角形有交点
								return true;
								//								cout<<"update"<<endl;
								//r.tMax=bestT;
								//这一步主要是想
								//减少tri.intersectTest(r)函数运算量
								//现在不用
							}
						} 


					}
					if(!isIntersect)
						pMB->rayID=currRayID;
					//说明此mailbox内的面与光线不相交	
					//或mailbox的包围盒与光线不相交
				}
			}
			//			cout<<setprecision(10)<<"bestT"<<bestT<<endl;
			//			cout<<setprecision(10)<<"oldTMax"<<oldTMax<<endl;
			//求交成功,且交点在节点（而不是mailbox）包围盒内
// 			if(bestT<r.tMax&&bestT<=tMax&&bestT>=tMin)
// 			{
// 				//				cout<<"**************find intersection************"<<endl;
// 				bestTriangle.intersect(r,DG);
// 				//				DG.position.showCoords();
// 				return true;
// 			}
// 			else//求交失败
// 			{
				//				cout<<"**************not find intersection************"<<endl;
			if(toDo.size()==0)//这表示光线与整个KD树都没有交点
				return false;
			WKDToDo nextToDo=toDo.back();
			toDo.pop_back();
			//				cout<<"pop"<<endl;
			pCurrNode=nextToDo.pNode;
			tMin=nextToDo.tMin;
			tMax=nextToDo.tMax;
				//				cout<<"min"<<tMin<<"max"<<tMax<<endl;
//			}
		}
	}
}
