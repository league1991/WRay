#pragma once

struct WRayGroup
{
	// 数组要求16字节对齐
	// AOS
	// ori_min = {(x y z min) (x y z min) ...}
	// dir_max = {(x y z max) (x y z max) ...}
	float* ori_min, *dir_max;

	// SOA 
	// ori[0] = {x x x...}
	// ori[1] = {y y y...}
	// ori[2] = {z z z...}
	float* ori[3];
	float* invDir[3];


	unsigned short numRays;
	WTriangle** isectTriangle;
	WDifferentialGeometry* DG;

	void allocSpace(short numRays);
	void freeSpace();
	~WRayGroup();
	WRayGroup();
};

struct WMultiRSBVHLeaf
{
	WTriangle* triangles[WBVHNode::maxTrianglesPerLeaf];
};

class WMultiRSBVH:public WMultiBVH
{
public:
	WMultiRSBVH();
	~WMultiRSBVH();
	void intersect( WRayGroup& rayGroup );	
	bool isIntersect(WRay& r, int beginNode = 0);
protected:
	void buildEnhancedTree();
	void buildEnhancedTreeKernel(int ithNode, int& currInterior, int& currLeaf);

	void displayNodeInfo();

private:
	void displayNodeKernel(WMultiBVHNode* pNode);
	void drawTreeRecursive( WMultiBVHNode* pNode );
	WMultiRSBVHLeaf* multiRSLeafs;
	struct WTask
	{
		union
		{
			WMultiBVHNode* pInterior;
			WMultiRSBVHLeaf* pLeaf;
		};
		int numRay;
		char SIMDlane;
		char isLeaf;
	};

};