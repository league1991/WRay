#pragma once
#include "Vector2.h"

class Vector3
{
public:
	//这样既可以通过 x y z 访问，也可以用数组访问
	union
	{
		struct
		{
			float x,y,z;
		};
		float v[4];
	};
//	float x,y,z;
	//构造函数
	Vector3();
	Vector3(float i);
	Vector3(const Vector3&v);
	Vector3(const Vector2f&v);
	Vector3(float ix,float iy,float iz);

	//析构函数
	Vector3::~Vector3(void);
	//加法
	Vector3 operator+(const Vector3&i)const;
	Vector3 operator+=(const Vector3&i);

	//减法
	Vector3 operator-(const Vector3&i)const;
	Vector3 operator-=(const Vector3&i);

	//数乘
	friend Vector3 operator*(const Vector3&i,float n);
	friend Vector3 operator*(float n,const Vector3&i);
	friend Vector3 operator*(const Vector3&i1,const Vector3&i2);

	Vector3 operator*=(float f);
	Vector3 operator*=(const Vector3&i);

	//数除
	friend Vector3 operator/(const Vector3&i,float n);
	friend Vector3 operator/(float n,const Vector3&i);
	friend Vector3 operator/(const Vector3&i,const Vector3&n);

	Vector3 operator/=(float f);
	Vector3 operator=(const Vector3&i);

	bool operator==(const Vector3& i)const;

	//长度
	float length()const;
	float lengthSquared()const;
	//单位化
	Vector3 normalize();
	//计算反射向量
	Vector3 reflect(const Vector3 normal)const;
	Vector3 sqrtElement()const;

	bool isZero() { return x == 0 && y == 0 && z == 0; }

	//点乘
	float dot(const Vector3&i)const;
	//叉乘
	Vector3 cross(const Vector3&i)const;

	//输出值，调试时用
	void showCoords()const;
};
