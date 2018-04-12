#pragma once
#include "Vector3.h"

class Vector4
{
public:
	float x,y,z,h;
	//构造函数
	Vector4();//xyz被初始化为0.0f，h为1.0f
	Vector4(float i);
	Vector4(const Vector4&v);
	Vector4(const Vector3&v);//h被初始化为1.0f
	Vector4(const Vector2&v);//h被初始化为1.0f
	Vector4(float ix,float iy,float iz,float ih);

	//析构函数
	Vector4::~Vector4(void);
	//加法
	Vector4 operator+(const Vector4&i);
	Vector4 operator+=(const Vector4&i);

	//减法
	Vector4 operator-(const Vector4&i);
	Vector4 operator-=(const Vector4&i);

	//数乘
	friend Vector4 operator*(const Vector4&i,float n);
	friend Vector4 operator*(float n,const Vector4&i);

	Vector4 operator*=(float f);

	//数除
	friend Vector4 operator/(const Vector4&i,float n);
	friend Vector4 operator/(float n,const Vector4&i);

	Vector4 operator/=(float f);

	//长度
	float length();
	float lengthSquared();
	//单位化
	Vector4 normalize();

	//点乘
	float dot(const Vector4&i);
};
