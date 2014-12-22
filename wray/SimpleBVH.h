#pragma once
#include "stdafx.h"
//定义是不是采用没有堆栈的求交方法
#define STACKLESS_TRAVERSAL
//#define BVH_MAX_TRIANGLES_PER_LEAF = 5;
#define SBVH_SUBTREE_DEPTH	2
struct WBVHNode
{
	//每个叶节点最多的三角形数,1 - 4 之间取值
	static const unsigned int maxTrianglesPerLeaf =	4;
	enum NodeType{
		BVHN_XSPLIT=0,BVHN_YSPLIT=1,BVHN_ZSPLIT=2,
		BVHN_LEAF=3};
	union
	{
		float box[6];							//包围盒
		WTriangle* tris[maxTrianglesPerLeaf];	//三角形索引
	};
	NodeType type;								//节点类型
	union
	{
		unsigned int farNode;					//右节点
		unsigned int nTriangles;				//三角形个数
	};

};
//压缩后的BVH节点，用来把它转存到显存
struct WBVHCompressedNode
{
	//每个叶节点最多的三角形数
	static const unsigned int maxTrianglesPerLeaf
		=WBVHNode::maxTrianglesPerLeaf;
	union
	{
		float box[6];							//包围盒
		WTriangle* tris[maxTrianglesPerLeaf];	//三角形索引
	};
	//onHit表示与包围盒相交时转到的节点索引
	//最高位同时存储节点类型，
	//为0时，表示为内部节点，否则为叶节点
	//真正的节点索引 = onHit & 0x7FFFFFFF
	//节点类型 = type & 0x80000000
	union
	{
		int onHit;	    //对于内部节点，表示命中时转到的节点索引
		int type;		//节点类型
		int nTris;		//对于叶节点，表示三角形数量
	};
	union
	{
		int next;		//对于叶节点，表示下一个应该访问的节点
		int onMiss;		//对于内部节点，表示不命中时转到的节点索引
	};
};
struct WBin
{
	WBoundingBox box;
	unsigned int nTriangles;
	WBin(){nTriangles=0;}
};


class WSimpleBVH:public WAccelerator
{
public:
	WSimpleBVH(void);
	~WSimpleBVH(void);
	// WAccelerator 提供的接口
	void buildTree(WScene&scene);
	void clearTree();
	void drawTreeIteractively();
	void drawTree(unsigned int nthBox=0,
				  float R = 0.7, 
				  float G = 0.7, 
				  float B = 0.7);//递归方法，用于检查节点
	bool intersect(WRay&r,WDifferentialGeometry&DG, int* endNode, int beginNode = 0);//求交函数
	bool isIntersect(WRay&r, int beginNode = 0);
	void displayNodeInfo();

	// WSimpleBVH 特有的接口
	virtual void displayEnhancedNodeInfo();

protected:
	//以下为常量参数，用于构建BVH树

	//每个节点构建的时候会先把三角形按照质心位置
	//放到对应的小格里面，小格的数量与三角形数量成正比，
	//等于三角形数量/binRatio,可见此值越小，精度越高，
	//但是构建时间也越长
	static const unsigned int binRatio = 4;
	//开销函数C=traversalCost+isectCost*(nL*SL+nR*SR)/SP;
	//traversalCost
	//isectCost
	//nL, nR为左右节点三角形个数
	//SL, SR，SP为左右节点及父节点包围盒面积
	//不过由于当每节点三角形数小于给定数才终止划分，
	//故函数可以进一步化简为C=nL*SL+nR*SR
	//static const float traversalCost = 1.0f;
	//static const float isectCost = 10.0f;
protected:
	WTriangle* triangles;				//场景三角形数组
	unsigned int nTriangles;			//三角形总个数
	WBoundingBox sceneBox;

	vector<WBVHNode>nodes;				//节点数组
	WBVHCompressedNode* compressedNodes;	//压缩的节点数组
	unsigned int nodeMaxDepth;			//记录BVH达到的最大深度

	//BVH树内核
	//triangleIDs是三角形的索引
	//box是节点对应的包围盒
	//当划分后出现一边节点三角形为0的情况时
	//即出现病态情况，这种情况可能是由于多个重叠的三角形造成的
	//此时直接新建一个与之储存一样的三角形的叶节点
	//和一个空叶节点
	virtual void buildTreeKernel(
		vector<unsigned int>& triangleIDs,
		WBoundingBox&box ,unsigned int depth=0,bool isIll=false);
	
	//根据已有的BVH，构建压缩的BVH树
	virtual void buildEnhancedTree();

	virtual void drawTreeRecursivelyKernel(vector<WBVHNode>&nodes,
									unsigned int ithNode);

	//根据索引获得三角形
	WTriangle&getTriangle(unsigned int nthTriangle)
	{return triangles[nthTriangle];}

	//获得节点的包围盒
	void getBBoxFromInterior(WBoundingBox& box, WBVHNode& node)
	{
		box.pMin.x = node.box[0];
		box.pMin.y = node.box[1];
		box.pMin.z = node.box[2];
		box.pMax.x = node.box[3];
		box.pMax.y = node.box[4];
		box.pMax.z = node.box[5];
	}
	void getBBoxFromCompressedNode(
		WBoundingBox& box, WBVHCompressedNode& node)
	{
		box.pMin.x = node.box[0];
		box.pMin.y = node.box[1];
		box.pMin.z = node.box[2];
		box.pMax.x = node.box[3];
		box.pMax.y = node.box[4];
		box.pMax.z = node.box[5];
	}



	//对于BVHCompressedNode的操作
	//检测是否为内部节点
	inline bool isInterior(WBVHCompressedNode& node)
	{
		return (node.type & 0x80000000) == 0;
	}
	inline int getOnHitID(WBVHCompressedNode& node)
	{
		return node.onHit & 0x7fffffff;
	}
	inline void setInterior(WBVHCompressedNode& node, int onHitID)
	{
		//把type最高位置0
		node.onHit = onHitID;
		node.type &= 0x7fffffff;
		//onMiss属性可以直接赋值，故不在函数里进行
	}
	inline void setLeaf(WBVHCompressedNode& node, int nTris)
	{
		node.nTris = nTris;
		//把type最高位置1
		node.type |= 0x80000000;
	}
	inline int getNTris(WBVHCompressedNode& node)
	{
		return node.nTris & 0x7fffffff;
	}

};
