#pragma once
#include "MailBox.h"
#include "Scene.h"
#include "Clock.h"
#include "Accelerator.h"
//KD树节点的结构
struct WKDNode
{
	enum NodeType{KDN_XSPLIT=0,KDN_YSPLIT=1,KDN_ZSPLIT=2,KDN_LEAF=3};
	NodeType type;
	union
	{
		float splitPos;				//内部节点：分割坐标的位置
		unsigned int nMailBoxes;				//叶节点：mailBox的个数
	};
	union
	{
		int aboveChild;				//内部节点：一个子节点在节点数组的位置
		int*mailBoxIndices;			//叶节点：mailBox数组的下标
	};
};
struct WKDToDo
{
	WKDNode*pNode;
	float tMin,tMax;
};

class WKDTree:public WAccelerator
{
	struct BoundingEdge
	{
		float t;
		unsigned int mailBoxNum;
		enum EdgeType{BE_START=0,BE_END=1};
		EdgeType type;
		friend bool operator<(const BoundingEdge&e1,const BoundingEdge&e2){return e1.t<e2.t;}
	};
public:
	WKDTree(void);
	virtual ~WKDTree(void);

	void cleanMailBoxes();			//把每个mailBox的rayID归0
	
	//画出mailbox对应SubPrimitive的包围盒,调试时用
	void drawMailBoxes(
		float R=0.4,
		float G=0.4,
		float B=0.4);			
	void buildTree(WScene&scene);	//由场景数据创建KD树
	//显示节点包围盒
	//此函数为递归调用，
	//可以依此检查节点信息是否正确
	void drawTree(unsigned int nthBox=0,
		float R=0.7,
		float G=0.7,
		float B=0.7);	//把节点画出来
	//显示整个场景
	//此函数为递归调用，
	//可以依此检查节点信息是否正确
	void drawScene(unsigned int nthBox=0,bool isFill=true);
	
	//显示节点的信息,调试时用，
	void displayNodeInfo();
	
	//求交函数，输入光线，输出DifferentialGeometry
	//返回布尔值表示求交成功与否
	bool intersect(WRay&r,WDifferentialGeometry&DG, 
		int& endNode, int beginNode = 0);
	void clearTree();//清除内存空间
	//检查光线是否在参数范围内跟场景的某个三角形相交
	bool isIntersect(WRay&r, int beginNode = 0);

	//设置节点的相关属性，应该在建立KD树之前设置这些属性
	void setNodeAttr(
		unsigned int imaxDepth=10,
		unsigned int imaxSubPsPerNode=3,
		float iisectCost=1.0f,
		float itraversalCost=10.0f
		);
	void displayTime();
protected:

	//mailbox的个数，也就是场景SubPrimitive的数量
	unsigned int nMailBoxes;		
	
	//已经分配了空间的KDNode个数，这个是指vector的元素个数
	//而不是mailBoxes数组的元素个数
	unsigned int nAllocatedNodes;	

	unsigned int nUsedNodes;		//已经被填充数据的KDNode个数		
	unsigned int currRayID;			//当前光线ID号，每求交一次，ID号自增
	unsigned int maxDepth;			//KD树最大深度
	unsigned int maxSubPsPerNode;	//每个节点包含的SubPrimitive最大数量，也就是mailBox的最大数量

	float traversalCost;			//用于确定最优划分的参数
	float isectCost;				//也是用于确定最优划分的参数

	float process;					//记录构建kd树的进度
	
	WKDNode*nodes;
	WMailBox*mailBoxes;
	WBoundingBox sceneBox;			//场景包围盒
	
	//各个节点的包围盒，每个节点对应一个包围盒，调试时用
	WBoundingBox*nodeBoxes;
	WClock clock;
	
	void buildMailBoxes(WScene&scene);//通过场景创建mailBox数组

	//用已建立好的mailbox创建KD树,整个场景定义为KD树的深度
	//为0的节点
	//boxes是当前节点包含的mailbox数组，nBoxes是数组元素个数
	//bBox是当前节点的包围盒,根节点的包围盒就是场景包围盒
	//调用此函数之前要分配
	void buildTreeCore(
		unsigned int depth,
		const vector<int>&mBoxIndices,
		const WBoundingBox&bBox,
		vector<WKDNode>&vNodes,
		vector<WBoundingBox>&vBoxes
		);

	//构造叶节点
	void makeLeaf(unsigned int nMBoxes,
		const vector<int>&mailBoxIndices,
		vector<WKDNode>&vNodes);
	//更新读取KD树的进度
	inline void updateProcess(const float &depth);
};
