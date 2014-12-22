#pragma once
#define PER_TRIANGLE_INTERSECT
#define PER_TRIANGLE_INTERSECT_TEST
#define MBVH_SUBTREE_DEPTH				1
struct WMultiBVHLeaf
{
	// 整个结构一共占176个字节
	// 加速三角形求交的数据,一共160字节

#if !defined(PER_TRIANGLE_INTERSECT) && !defined(PER_TRIANGLE_INTERSECT_TEST)
	__m128 nu,nv,np,
		   pu,pv,
		   e0u,e0v,e1u,e1v;
	__m128i ci;
#endif
	// 三角形指针，一共16字节
	WTriangle* triangle[4];
};
struct WMultiRSBVHLeaf;
struct WMultiBVHNode
{
	// 四个包围盒，一个__m128变量存放四个包围盒一个分量的坐标
	// 若包围盒对应的节点不存在，则包围盒设为非法值（体积为0）
	__m128 bMin[3];
	__m128 bMax[3];
	// 到子节点或者叶节点的指针
	union
	{
		WMultiBVHNode* child[4];
		WMultiBVHLeaf* leaf[4];
		WMultiRSBVHLeaf* rsLeaf[4];
	};
	// 预先计算出的求交顺序，针对光线方向所在卦限，有8种情况
	unsigned char traversalOrder[8];
	// 标志位，表示对应子节点的类型，1为叶节点，0为内部节点
	char isLeaf[4];
};

class WMultiBVH: public WSimpleBVH
{
public:
	WMultiBVH();
	~WMultiBVH();
	// WAccelerator 提供的接口
	bool intersect(WRay& r,WDifferentialGeometry& DG, 
		int* endNode = NULL, int beginNode = -1);	
	bool isIntersect(WRay& r, int beginNode = -1);
	void clearTree();
	void drawTree(unsigned int nthBox=0,
		float R = 0.7, 
		float G = 0.7, 
		float B = 0.7);
	void displayNodeInfo();

protected:
	// 重载SimpleBVH的构建函数,这里用于构建四叉树形式的BVH
	void buildEnhancedTree();

	struct StackNode
	{
		WMultiBVHNode* node;
		float t;
	};
	WMultiBVHNode* multiInteriors;			// 内部节点数组
	WMultiBVHLeaf* multiLeafs;				// 叶节点数组
	int numInteriors;						// 内部节点数
	int numLeafs;							// 叶节点数


	// 构建四叉树节点
	// ithNode		表示根据原来的第几个节点构建节点
	// currInterior 表示当前已新建的内部节点数量
	// currLeaf		表示当前已新建的叶节点数量
	void buildEnhancedTreeKernel(int ithNode, 
		int& currInterior, int& currLeaf, int curDepth = 0);

	// 重新排列节点使得缓存命中率提高
	void buildCacheFriendlyKernel();

	// 设置节点包围盒， ithChild指定子节点序号
	inline void setBBoxToNode(WMultiBVHNode& node, const WBoundingBox& bBox, int ithChild);

	// 根据原来的叶节点设置四叉树的叶节点
	void makeLeaf(WMultiBVHLeaf& dstLeaf, WBVHNode& srcLeaf);

	// 根据节点的情况预先计算出表示求交顺序的标志位
	// srcNode 必须为内部节点
	void computeTraversalOrder(WMultiBVHNode& dstNode, 
		const WBVHNode& srcParent, 
		const WBVHNode& srcLeft, 
		const WBVHNode& srcRight);

	// 获得子节点三角形的包围盒
	void getBBoxFromSrcLeaf(WBVHNode& leaf, WBoundingBox& box);

	// 画出整棵树,被drawTree调用
	virtual void drawTreeRecursive(WMultiBVHNode* node);

	// 当子节点为空时，其父节点会存储这样一个无效的包围盒
	// 这个包围盒永远不会跟任何光线相交
	static const WBoundingBox invalidBox;		
};