#pragma once
#include "simplekd.h"
#define EKD_SUBTREE_DEPTH 2
struct WEKDInterior : public WSKDNode
{
	// 把三层的内部节点打包在一起
	// etype   [H][L] --> [6 5 4 3][2 1 0 x]
	union
	{
		unsigned short etype;			// 节点分隔平面情况
		unsigned char  etype_c[2];			
	};
	float splitPlane[7];			// 分隔平面
	WSKDNode* child[8];				// 子节点
};

class WEnhancedKD :
	public WSimpleKD
{
public:
	WEnhancedKD(void);
	~WEnhancedKD(void);
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
	WEKDInterior* ekdInteriors;			// 增强的内部节点数组
	int numEInteriors;					// 内部节点数

	// 构建打包的内部节点
	void buildEnhancedInterior();		

	//构建 ropes, 后6个参数分别代表节点包围盒6个面的相邻节点索引

};
