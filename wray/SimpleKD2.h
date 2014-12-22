#pragma once
//#define SKD_RECOMM_LEAF_TRIS			3
#define SKD_MAX_DEPTH					70
#define SKD_MIN_BOX_FACTOR				5e-4
//#define SKD_SUBTREE_DEPTH				3

#define SKD_INTERIOR_MASK				0x80000000
#define SKD_LEAF_MASK					0x40000000
//叶子节点包含的三角形数量
struct WSKDNode2
{
	enum NodeType {KDN_XSPLIT=0,KDN_YSPLIT=1,KDN_ZSPLIT=2,KDN_LEAF=3};
	unsigned char type;
};
//内部节点
struct WSKDInterior2 : public WSKDNode2
{
	float splitPlane;
	WSKDNode2* child[2];
};

//叶节点
struct WSKDLeaf2 : public WSKDNode2
{
//	WBoundingBox box;
	//分别为 pMin.x pMin.y pMin.z pMax.x pMax.y pMax.z
	float box[2][3];
	//到相邻节点的连接,分别为 
	//x 正面， x负面， y 正面， y负面， z 正面， z负面 
	WSKDNode2* ropes[2][3];
	//WTriangle** triangle;
	int nTriangles;
	TriAccel* triInfo;
};

class WSimpleKD2:public WAccelerator
{
	//Bounding Edge 用于把三角形按照包围盒分成两堆
protected:
	struct WBoundingEdge
	{
		enum EdgeType{BE_END = 0,BE_PLANAR = 1, BE_START = 2};
		enum EdgeAxis{BE_X = 0,BE_Y = 1,BE_Z = 2};
		//BE的类型，
		//分别为三角形的起始点，终止点
		WBoundingEdge(WBoundingEdge::EdgeType t, EdgeAxis a, float pos, int triID):type(t), axis(a), t(pos), triangleID(triID){}
		WBoundingEdge(){}

		char type;
		char axis;
		float t;
		int   triangleID;

		//对三角形排序
		//排序确保BE按照t值排序
		//相同t的BE按照BE的类型和三角形ID的关系排序
		bool operator<(const WBoundingEdge&e)const;
	};

public:
	WSimpleKD2(void);
	~WSimpleKD2(void);
	void buildTree(WScene&scene);
	void clearTree();

	bool intersect(WRay& r,WDifferentialGeometry& DG, 
		int* endNode = NULL, int beginNode = -1);
	bool intersect4(WRay r[4], WDifferentialGeometry DG[4], int endNode[4] = NULL, int beginNode[4] = NULL);

	bool isIntersect(WRay& r, int beginNode = -1);

	void drawTree(unsigned int nthBox=0,
					float R = 0.7, 
					float G = 0.7, 
					float B = 0.7);
	void displayNodeInfo();
	void drawTriangles();
protected:
	WTriangle* triangles;				// 场景三角形数组
	unsigned int totalTriangles;		// 三角形总个数
	float sceneBox[2][3];				// 场景包围盒

	int treeDepth;						// 记录KD树达到的最大深度

	WSKDInterior2* kdInteriors;			// 内部节点数组
	WSKDLeaf2*     kdLeafs;				// 叶节点数组
	WTriangle** leafTriangles;			// 叶节点的三角形指针数组
	int numInteriors;					// 内部节点数
	int numLeafBytes;						// 叶节点数
	int numLeafTriangles;				// 当前数量

	int numInteriorsAllocated;
	int numLeafBytesAllocated;
	int numLeafTrianglesAllocated;

	float KT;							// 经过一个节点的开销
	float KI;							// 对一个三角形求交的开销

	float minBoxSize[3];				// 最小的叶节点大小，用于递归建树的终止条件
	float minBoxFactor;					// 最小叶节点大小与场景包围盒的比例

	static const char otherAxis[3][2];

	vector<WBoundingBox> nodeBoxes;		// 节点包围盒数组

	//构建 ropes, 后6个参数分别代表节点包围盒6个面的相邻节点索引
	void buildBasicRopes(WSKDNode2* ithNode, 
		WSKDNode2* ropeX_P, WSKDNode2* ropeX_N,
		WSKDNode2* ropeY_P, WSKDNode2* ropeY_N,
		WSKDNode2* ropeZ_P, WSKDNode2* ropeZ_N);

	//构建加强的 ropes
	void buildExtendedRopes();
	//判断 refBox 的指定面是否完全在 testBox 的一面上
	bool isOverlap(WBoundingBox& refBox,
				   WBoundingBox& testBox,
				   int refFace);

	void buildTreeKernel( vector<WBoundingEdge>&edges, 
		int nTris,
		int depth, 
		float nodeBox[2][3],
		char triangleMap[],
		int parentIndex,
		char branch);

	void drawTreeRecursive( WSKDNode2* node, const WBoundingBox& box );

	//对于一条与包围盒相交的光线，计算出光线从包围盒哪个面上离开
	int computeExitFace( WSKDLeaf2& node, const WRay& r, WVector3& exitPoint, float& farT);

	void buildCacheFriendlyNode();

	// 申请分配相应的空间
	WSKDInterior2* requestInterior();
	WSKDLeaf2* requestLeaf(int nTri);
	WTriangle** requestLeafTriangles(int nTri);
};
