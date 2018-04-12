#pragma once
#include "Vector3.h"
#include "MathValues.h"
#define RAY_EPSILON 1e-4f
class WRay
{
public:
	Vector3 point;
	Vector3 direction;
	float tMin,tMax;			//t为有效参数范围
	WRay(void);
	//注意iDirection向量是直接传给方向向量的
	WRay(const Vector3&iDirection);
	WRay(const Vector3&iPoint,const Vector3&iDirection);
	WRay(const Vector3&iPoint,const Vector3&iDirection,float iTMin,float iTMax);
	virtual ~WRay(void);
	void normalizeDir();	//把方向向量单位化
	void inverseDir();		//反转光线方向
	void reflect(Vector3 normal);//计算镜面反射的光线方向
	void draw();			//在openGL画出光线，调试时用
	void drawSegment();		//在openGL画出光线参数范围内的一段，调试时用
};
