#pragma once

class Vector2f
{
public:
	float x;
	float y;
	//构造函数	
	Vector2f();
	Vector2f(float i);
	Vector2f(const Vector2f&v);
	Vector2f(float ix,float iy);

	//析构函数
	Vector2f::~Vector2f(void){}
	//加法
	Vector2f operator+(const Vector2f&i2);
	Vector2f operator+=(const Vector2f&i2);

	//减法
	Vector2f operator-(const Vector2f&i2);
	Vector2f operator-=(const Vector2f&i2);

	//数乘
	friend Vector2f operator*(const Vector2f&i1,float n);
	friend Vector2f operator*(float n,const Vector2f&i1);

	Vector2f operator*=(float f);

	//数除
	friend Vector2f operator/(const Vector2f&i1,float n);
	friend Vector2f operator/(float n,const Vector2f&i1);

	Vector2f operator/=(float f);

	//长度
	float length();
	float lengthSquared();

	//单位化
	Vector2f normalize();

	//点乘
	float dot(const Vector2f&i);

	//输出值，调试时用
	void showCoords();
};

class Vector2i
{
public:
	int m_x;
	int m_y;
	Vector2i() {}
	Vector2i(int x, int y) :m_x(x), m_y(y) {}
};