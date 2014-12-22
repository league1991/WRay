#pragma once

class WVector2
{
public:
	float x;
	float y;
	//构造函数	
	WVector2();
	WVector2(float i);
	WVector2(const WVector2&v);
	WVector2(float ix,float iy);

	//析构函数
	WVector2::~WVector2(void){}
	//加法
	WVector2 operator+(const WVector2&i2);
	WVector2 operator+=(const WVector2&i2);

	//减法
	WVector2 operator-(const WVector2&i2);
	WVector2 operator-=(const WVector2&i2);

	//数乘
	friend WVector2 operator*(const WVector2&i1,float n);
	friend WVector2 operator*(float n,const WVector2&i1);

	WVector2 operator*=(float f);

	//数除
	friend WVector2 operator/(const WVector2&i1,float n);
	friend WVector2 operator/(float n,const WVector2&i1);

	WVector2 operator/=(float f);

	//长度
	float length();
	float lengthSquared();

	//单位化
	WVector2 normalize();

	//点乘
	float dot(const WVector2&i);

	//输出值，调试时用
	void showCoords();
};
