#pragma once
#include "Vector3.h"

class WVector4
{
public:
	float x,y,z,h;
	//构造函数
	WVector4();//xyz被初始化为0.0f，h为1.0f
	WVector4(float i);
	WVector4(const WVector4&v);
	WVector4(const WVector3&v);//h被初始化为1.0f
	WVector4(const WVector2&v);//h被初始化为1.0f
	WVector4(float ix,float iy,float iz,float ih);

	//析构函数
	WVector4::~WVector4(void);
	//加法
	WVector4 operator+(const WVector4&i);
	WVector4 operator+=(const WVector4&i);

	//减法
	WVector4 operator-(const WVector4&i);
	WVector4 operator-=(const WVector4&i);

	//数乘
	friend WVector4 operator*(const WVector4&i,float n);
	friend WVector4 operator*(float n,const WVector4&i);

	WVector4 operator*=(float f);

	//数除
	friend WVector4 operator/(const WVector4&i,float n);
	friend WVector4 operator/(float n,const WVector4&i);

	WVector4 operator/=(float f);

	//长度
	float length();
	float lengthSquared();
	//单位化
	WVector4 normalize();

	//点乘
	float dot(const WVector4&i);
};
