#pragma once

//叶子节点包含的三角形数量

struct WSKDNode
{
	static const unsigned int maxTrianglesPerLeaf =	5;
	enum NodeType {KDN_XSPLIT=0,
		KDN_YSPLIT=1,KDN_ZSPLIT=2,KDN_LEAF=3};
	inline bool isInterior(){return type != KDN_LEAF;}
	NodeType type;
};
//内部节点
struct WSKDInterior : public WSKDNode
{
	float splitPlane;
	int leftChild;
	int rightChild;
};
//叶节点
struct WSKDLeaf : public WSKDNode
{
//	WBoundingBox box;
	//分别为 pMin.x pMin.y pMin.z pMax.x pMax.y pMax.z
	float box[6];
	//到相邻节点的连接,分别为 
	//x 正面， x负面， y 正面， y负面， z 正面， z负面 
	int ropes[6];		
	int triangleIDs[maxTrianglesPerLeaf];
	int nTriangles;
};

class WSimpleKD:public WAccelerator
{
	//Bounding Edge 用于把三角形按照包围盒分成两堆
	struct BoundingEdge
	{
		//BE的类型，
		//分别为三角形的起始点，终止点
		enum EdgeType{
			  BE_END = 0,BE_PLANAR = 1, BE_START = 2};

		EdgeType type;
		float t;
		int   triangleID;

		//对三角形排序
		//排序确保BE按照t值排序
		//相同t的BE按照BE的类型和三角形ID的关系排序
		bool operator<(const BoundingEdge&e)const;
	};

public:
	WSimpleKD(void);
	~WSimpleKD(void);
	void buildTree(WScene&scene);
	bool intersect(WRay& r,WDifferentialGeometry& DG, 
		int* endNode = NULL, int beginNode = 0);
	void clearTree();
	bool isIntersect(WRay& r, int beginNode = 0);
	void drawTree(unsigned int nthBox=0,
					float R = 0.7, 
					float G = 0.7, 
					float B = 0.7);
	void displayNodeInfo() {};
	void drawTriangles();
private:
	WTriangle* triangles;				//场景三角形数组
	unsigned int totalTriangles;			//三角形总个数
	WBoundingBox sceneBox;				//场景包围盒

	vector<WSKDNode*> nodes;				//节点数组
	vector<WBoundingBox> nodeBoxes;		//节点包围盒数组
	int nodeMaxDepth;					//记录KD树达到的最大深度

	//构建 ropes, 后6个参数分别代表节点包围盒6个面的相邻节点索引
	void buildBasicRopes(int ithNode, 
						int ropeX_P, int ropeX_N,
						int ropeY_P, int ropeY_N,
						int ropeZ_P, int ropeZ_N);

	//构建加强的 ropes
	void buildExtendedRopes();
	//判断 refBox 的指定面是否完全在 testBox 的一面上
	bool isOverlap(WBoundingBox& refBox,
				   WBoundingBox& testBox,
				   int refFace);
	//检查并更新叶节点的一个面
	int checkFace(WBoundingBox& refBox,
				  int& rope, int refFace);
	//找到一个跟当前包围盒的
	void buildTreeKernel(vector<BoundingEdge>& edgesX, 
						vector<BoundingEdge>& edgesY, 
						vector<BoundingEdge>& edgesZ,
						WBoundingBox bBox, 
						int* triangleMark, 
						int nTriangles, int depth);

	//计算分割平面的最佳位置，
	//splitType指定 bounding edge 的方向
	//注意，此函数要求同时提供一个三角形的
	//开始 BE 和结束 BE ,或者提供一个平面 BE
	//也就是 bounding edge 的个数为偶数
	//同时，此函数要求 bounding edge 的元素个数不为 0
	//isLeft参数表示如果最优的分隔平面为一个平面类型BE，
	//其对应的三角形是否在左边
	void computeBestSplit(vector<BoundingEdge>& edges,
						  WSKDNode::NodeType splitType,
						  WBoundingBox& bBox,
						  int nTriangles,
						  int& nTriangles_L,
						  int& nTriangles_R,
						  int& bestPosition,
						  bool& isLeft,
						  float& bestT
						  );

	//根据分割平面的位置
	//确定三角形的类型
	//只在左边的三角形对应标记为 2
	//只在右边的三角形对应标记为-2
	//横跨节点的三角形对应标记为 0 ,
	//对应索引加入 middleTriangleID
	//isLeft参数表示三角形是否在平面类型BE的左边
	void markTriangles(vector<BoundingEdge>& refEdges,
						vector<int>& middleTriangleID,
						int bestBE,
						int nTriangles,
						int* triangleMark,
						bool isLeft);

	//根据横跨分割平面的三角形计算其在子节点包围盒内部的BE
	//由于平面BE的存在，所以三个方向的middleBE数量可能不相等
	void getNewSortedBE(vector<int>& middleTriangleID,
				  const WBoundingBox& clipBox,
				  vector<BoundingEdge>& middleBEX,
				  vector<BoundingEdge>& middleBEY,
				  vector<BoundingEdge>& middleBEZ);

	//根据标记了的三角形类型
	//以及新创建的BE得到左右节点的BE
	//oldEdge 为原有的BE
	//middleEdge为新创建的BE
	//newEdge为合并以后的BE，作为子节点的新BE
	void mergeBE(vector<BoundingEdge>& oldEdge, 
				vector<BoundingEdge>& middleEdge_L, 
				vector<BoundingEdge>& middleEdge_R, 
				vector<BoundingEdge>& newEdge_L, 
				vector<BoundingEdge>& newEdge_R, 
				int nTriangles_L, 
				int nTriangles_R, 
				int* triangleMark);

	void buildLeaf(WBoundingBox bBox,
					vector<BoundingEdge>& edges);
	void drawTreeRecursive(int nthNode, const WBoundingBox& box);
	//根据参数值添加Bounding Edge,
	//如果三角形的包围盒两面重合，只添加一个 BE_PLANAR 类型的BE
	//否则，添加 BE_START 和 BE_END 类型的BE
	void addEdge( vector<BoundingEdge>& edges, 
				float minT, float maxT, 
				int ithTriangle );

	//开销函数，用于确定最佳的分隔
	inline float computeCost(float& emptyFactor,
							  float& lengthL, float& lengthR,
							  float& nBelow, float& nAbove)
	{	return emptyFactor * 
			(lengthL * nBelow + lengthR * nAbove);}

	//计算比例因数，有一侧为空时，
	//如果分隔平面不在包围盒表面上
	//适当减小比例因数，
	//反正增大比例因数
	inline float computeEmptyFtr(float nBelow, float nAbove,
								 float nTriangles,
								 float& t, 
								 float& tMin, float& tMax)
	{
		return (((nBelow == 0.0f) || (nAbove == 0.0f))?
			((t > tMin  && t < tMax)? 0.8f: 5.0f):
			1.0f);
	}
	//记录最优划分下两边的三角形数量
	inline void updateBest(int& bestBE, int& nthBE, 
					float& bestCost, float& cost,
					int& nTriangles_L, int& nTriangles_R,
					int nBelow, int nAbove)
	{
		bestBE = nthBE;		bestCost = cost;
		nTriangles_L = nBelow;		nTriangles_R = nAbove;
	}


	//对于一条与包围盒相交的光线，计算出光线从包围盒哪个面上离开
	int computeExitFace( WSKDLeaf& node, const WRay& r, WVector3& exitPoint, float& farT);

};
