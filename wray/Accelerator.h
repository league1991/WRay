#pragma once

class WAccelerator
{
public:
	WAccelerator(void);
	virtual ~WAccelerator(void);
	//求交函数，输入光线，返回表示相交表面属性的DG
	//beginNode为开始搜索时所在的节点
	//endNode为求交成功时的节点，
	//这两个变量是SimpleKD树专用的，用于加快求交速度
	//KDTree SimpleBVH 均不需要使用这两个变量
	virtual bool intersect(WRay& r,WDifferentialGeometry& DG, 
						int* endNode = NULL, int beginNode = -1)=0;	
	virtual bool isIntersect(WRay& r, int beginNode = 0) = 0;
	virtual void buildTree(WScene& scene) = 0;
	virtual void clearTree() = 0;
	virtual void drawTree(unsigned int nthBox=0,
							float R = 0.7, 
							float G = 0.7, 
							float B = 0.7) = 0;
	virtual void displayNodeInfo() = 0;
	virtual void resetStatistics();
	virtual void displayIsectStatistics();
	// 按照16字节边界分配和释放数组
	static void* allocAligned16(const unsigned int bytes);
	static void  freeAligned16(void* ptr);
protected:
	// 以下为统计求交数据的变量
	int numIntersectTest;					// 总求交测试数
	int numIntersect;						// 总求交数

	int numTraverseInterior_IT;				// 内部节点遍历总数（求交测试）
	int numTraverseLeaf_IT;					// 叶节点遍历总数（求交测试）
	int numTriangleIsect_IT;				// 三角形求交总数(求交测试)

	int numTraverseInterior_I;				// 内部节点遍历总数（求交）
	int numTraverseLeaf_I;					// 叶节点遍历总数（求交）
	int numTriangleIsect_I;					// 三角形求交总数(求交)

};
