#pragma once

class Vector2
{
public:
	float x;
	float y;
	//构造函数	
	Vector2();
	Vector2(float i);
	Vector2(const Vector2&v);
	Vector2(float ix,float iy);

	//析构函数
	Vector2::~Vector2(void){}
	//加法
	Vector2 operator+(const Vector2&i2);
	Vector2 operator+=(const Vector2&i2);

	//减法
	Vector2 operator-(const Vector2&i2);
	Vector2 operator-=(const Vector2&i2);

	//数乘
	friend Vector2 operator*(const Vector2&i1,float n);
	friend Vector2 operator*(float n,const Vector2&i1);

	Vector2 operator*=(float f);

	//数除
	friend Vector2 operator/(const Vector2&i1,float n);
	friend Vector2 operator/(float n,const Vector2&i1);

	Vector2 operator/=(float f);

	//长度
	float length();
	float lengthSquared();

	//单位化
	Vector2 normalize();

	//点乘
	float dot(const Vector2&i);

	//输出值，调试时用
	void showCoords();
};
