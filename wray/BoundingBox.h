#pragma once
#include "Vector3.h"
#include "Triangle.h"

class WBoundingBox
{
public:
	static float delta;
	WVector3 pMin;
	WVector3 pMax;

	WBoundingBox(void);
	WBoundingBox(const WTriangle&it, bool isBigger = true);
	WBoundingBox(WVector3 ipMin,WVector3 ipMax);
	WBoundingBox(float box[2][3]);
	virtual ~WBoundingBox(void);
	//所有方法都把bbox设得比模型大一点
	void merge(const WVector3&point,bool isBigger = true);
	void merge(const WBoundingBox&iBox);
	void merge(const WTriangle&t,bool isBigger = true);
	//求与另一个bbox的相交部分
	bool intersect(const WBoundingBox& iBox, WBoundingBox& resultBox);
	//获得模型的实际bbox坐标，而不是设大了的bbox
	void getTightBound(WVector3& tightMin, WVector3& tightMax);
	void draw()const;
	//返回一半的面积
	float halfArea();
	float area();
	//显示坐标值
	void displayCoords()const;
	//光线与包围盒求交，返回参数范围（可能为负数）
	bool isIntersect(const WRay&r,
		float&tMin,float&tMax);
	//检测一个点是不是在包围盒里面，包括刚好在包围盒表面上的点
	bool isInBoxInclusive(const WVector3& point);
	//检测一个点是不是在包围盒里面，不含刚好在包围盒表面上的点
	bool isInBoxExclusive(const WVector3& point);
	//对于一个已经确定不在bbox外面的点
	//确定它是否在一个面上，如果在，在哪一个面上面
	//face表示面的序号
	// 0 为x正面， 1 为x负面
	// 2 为y正面， 3 为y负面
	// 4 为z正面， 5 为z负面
	bool onFace(WVector3& point, int& face);
	//检查光线是否与包围盒相交
	//找出坐标范围最大的轴向,x方向为0，y方向为1，z方向为2
	//tMin,tMax为光线交点的参数值
	int maxAxis();	
	WBoundingBox operator=(const WBoundingBox&box);
	bool operator== (const WBoundingBox& box)const;
private:
	inline void swap(float&t1,float&t2);
};
