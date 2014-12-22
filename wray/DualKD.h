#pragma once
#include "simplekd.h"
#define DKD_SUBTREE_DEPTH 3
struct WDKDInterior : public WSKDNode
{
	// 把2层的内部节点打包在一起
	char dtype[3];					// 类型
	float splitPlane[3];			// 分隔平面
	WSKDNode* child[4];				// 子节点
};

class WDualKD :public WSimpleKD
{
public:
	WDualKD(void);
	~WDualKD(void);
	void buildTree(WScene&scene);
	void clearTree();
	
	bool intersect(WRay& r,WDifferentialGeometry& DG, 
		int* endNode = NULL, int beginNode = -1);
	bool isIntersect(WRay& r, int beginNode = -1);

	void drawTree(unsigned int nthBox=0,
		float R = 0.7, 
		float G = 0.7, 
		float B = 0.7);
	void displayNodeInfo();
	void drawTriangles();

private:
	WDKDInterior* ekdInteriors;			// 增强的内部节点数组
	int numEInteriors;					// 内部节点数

	// 构建打包的内部节点
	void buildEnhancedInterior();		

	//构建 ropes, 后6个参数分别代表节点包围盒6个面的相邻节点索引

};
