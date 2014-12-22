#include "StdAfx.h"
#include "OctTree.h"
void WIrradianceSample::display(bool isDisplayBox)
{
	glColor3f(E.x,E.y,E.z);
	glPointSize(3.0f);
	glBegin(GL_POINTS);
	glVertex3f(point.x,point.y,point.z);
	glEnd();
	if(!isDisplayBox)
		return;
	WVector3 radius=WVector3(maxDist);
	WBoundingBox box;
	box.pMin=point-radius;
	box.pMax=point+radius;
	box.draw();
}
void WIrradianceInterpolator::operator ()(WIrradianceSample&sample)
{
	nSamplesChecked++;
	float cosTheta=normal.dot(sample.normal);
	if(cosTheta<maxNormalError)
	{
//		cout<<"normal"<<endl;
		return;
	}
	float distance=(position-sample.point).length();
	if(distance>sample.maxDist)
	{
//		cout<<"distance"<<endl;
		return;
	}
//	cout<<1;
	WVector3 delta=sample.point-position;
	delta.normalize();
	if(abs(delta.dot(normal))>maxPlanarError)
	{
//		cout<<"planar"<<endl;
		return;
	}
//	cout<<2;
	float error=distance/(sample.maxDist*cosTheta*smoothFactor);
//	cout<<(error)<<endl;
	if(error<1.0f)
	{
//		cout<<3;
		//cout<<"succeed"<<endl;
		nSamplesAccepted++;
//		cout<<nSamplesAccepted<<endl;
		float weight=(1.0f-error)*(1.0f-error);
		totalE+=weight*sample.E;
		totalWeight+=weight;
//		cout<<totalWeight<<endl;
	}
// 	totalE.showCoords();
// 	cout<<totalWeight<<endl;
}
bool WIrradianceInterpolator::finalInterpolate(WVector3&E)
{
//	cout<<nSamplesAccepted<<endl;
	if(nSamplesAccepted<minSamples)
	{
//		cout<<"fail"<<endl;
		return false;
	}
//	cout<<"success";
//	cout<<totalWeight<<ends;
//	totalE.showCoords();
	E=totalE/totalWeight;
//	E.showCoords();
	return true;
}
WOctNode::WOctNode()
{
	for(int i=0;i<8;i++)
		children[i]=NULL;
}
WOctTree::WOctTree(WScene*iscene,unsigned int imaxDepth):
scene(iscene)
{
	sceneBox=scene->getBBox();
	maxDepth=imaxDepth;
	root=new WOctNode;
}

WOctTree::~WOctTree(void)
{
	clear();
}
void WOctTree::initialize(WScene*iscene)
{
	scene=iscene;
	root=new WOctNode;
}
void WOctTree::clear()
{
	boxes.clear();
	deleteNodes(root);
	delete root;
	root=new WOctNode;
	sceneBox=scene->getBBox();
// 	delete root;
// 	root=NULL;
}
void WOctTree::deleteNodes(WOctNode*node)
{
	for(int i=0;i<8;i++)
	{
		if(node->children[i])
		{
//			cout<<'d'<<endl;
			//清理子节点
			deleteNodes(node->children[i]);
			//当子节点清理完毕后，删除这个节点
			delete node->children[i];
		}		
	}
	//清理当前节点的数据
	node->samples.clear();
}
void WOctTree::addSample(const WIrradianceSample&sample)
{
	WVector3 point=sample.point;
	WVector3 radius(sample.maxDist);
//	radius.showCoords();
	WBoundingBox sampleBox;
	sampleBox.pMin=point-radius;
	sampleBox.pMax=point+radius;
	add(root,sample,sceneBox,sampleBox,
		(sampleBox.pMax-sampleBox.pMin).lengthSquared(),0);

}
void WOctTree::add(
				  WOctNode*node,
				  const WIrradianceSample&sample,
				  WBoundingBox nodeBox, 
				  const WBoundingBox&sampleBox, 
				  float diagSquared,
				  unsigned int depth)
{
//	cout<<"add"<<endl;
	WVector3 diag=nodeBox.pMax-nodeBox.pMin;
//	diag.showCoords();
	if(depth==maxDepth||
		diag.lengthSquared()<diagSquared)
	{
//		cout<<"add"<<endl;
		node->samples.push_back(sample);
		return;
	}
	WVector3 center=0.5f*(nodeBox.pMin+nodeBox.pMax);
	//确定采样点的包围盒跟那些子节点相交
	bool over[8];
	over[0]=over[1]=over[2]=over[3]=
		(sampleBox.pMin.z<=center.z);
	over[4]=over[5]=over[6]=over[7]=
		(sampleBox.pMax.z>center.z);

//	cout<<over[0];
	bool pMinY=sampleBox.pMin.y<=center.y;
	over[0]&=pMinY;
	over[1]&=pMinY;
	over[4]&=pMinY;
	over[5]&=pMinY;

	bool pMaxY=sampleBox.pMax.y>center.y;
	over[2]&=pMaxY;
	over[3]&=pMaxY;
	over[6]&=pMaxY;
	over[7]&=pMaxY;

	bool pMinX=sampleBox.pMin.x<=center.x;
	over[0]&=pMinX;
	over[2]&=pMinX;
	over[4]&=pMinX;
	over[6]&=pMinX;

	bool pMaxX=sampleBox.pMax.x>center.x;
	over[1]&=pMaxX;
	over[3]&=pMaxX;
	over[5]&=pMaxX;
	over[7]&=pMaxX;

	for(int nthChild=0;nthChild<8;nthChild++)
	{
		if(!over[nthChild])
		{
//			cout<<"notOver"<<endl;
			continue;
		}
//		cout<<"over"<<endl;
		if(!node->children[nthChild])
			node->children[nthChild]=new WOctNode;

		WBoundingBox childBox;
		childBox.pMin.x=(nthChild&1)?center.x:nodeBox.pMin.x;
		childBox.pMax.x=(nthChild&1)?nodeBox.pMax.x:center.x;
		childBox.pMin.y=(nthChild&2)?center.y:nodeBox.pMin.y;
		childBox.pMax.y=(nthChild&2)?nodeBox.pMax.y:center.y;
		childBox.pMin.z=(nthChild&4)?center.z:nodeBox.pMin.z;
		childBox.pMax.z=(nthChild&4)?nodeBox.pMax.z:center.z;

		//把包围盒加入包围盒数组，调试时用,最终渲染可以去掉
//		boxes.push_back(childBox);
		add(node->children[nthChild],sample,childBox,sampleBox,diagSquared,depth+1);
	}

}
void WOctTree::displayNodes()
{
	vector<WBoundingBox>::iterator p;
	for(p=boxes.begin();p!=boxes.end();p++)
	{
		p->draw();
	}
}
void WOctTree::displaySample(WOctNode*node,bool isDisplayBox)
{
//	cout<<node->samples.size();
	for(unsigned int i=0;i<node->samples.size();i++)
	{
		node->samples[i].display(isDisplayBox);
	}
	for(int i=0;i<8;i++)
	{
		if(node->children[i])
		{
			displaySample(node->children[i]);
		}
	}
}
void WOctTree::displaySamples(bool isDisplayBox)
{
	displaySample(root,isDisplayBox);
}

void WOctTree::lookUP( WOctNode*node, WBoundingBox&nodeBox, WIrradianceInterpolator&interpolate )
{
	//处理当前节点的采样点
	for(unsigned int i=0;i<node->samples.size();i++)
	{
		interpolate(node->samples[i]);
	}
	WVector3 center=0.5f*(nodeBox.pMin+nodeBox.pMax);
	WVector3 point=interpolate.getPosition();
	//检测是在哪个子节点里面
	int nthChild=
		((point.z>center.z)?4:0)+
		((point.y>center.y)?2:0)+
		((point.x>center.x)?1:0);
	//如果还有子节点，处理子节点
	if(node->children[nthChild])
	{
		WBoundingBox childBox;
		childBox.pMin.x=(nthChild&1)?center.x:nodeBox.pMin.x;
		childBox.pMax.x=(nthChild&1)?nodeBox.pMax.x:center.x;
		childBox.pMin.y=(nthChild&2)?center.y:nodeBox.pMin.y;
		childBox.pMax.y=(nthChild&2)?nodeBox.pMax.y:center.y;
		childBox.pMin.z=(nthChild&4)?center.z:nodeBox.pMin.z;
		childBox.pMax.z=(nthChild&4)?nodeBox.pMax.z:center.z;
		lookUP(node->children[nthChild],childBox,interpolate);
	}
}
void WOctTree::process(WIrradianceInterpolator&interpolate)
{
	lookUP(root,sceneBox,interpolate);
}