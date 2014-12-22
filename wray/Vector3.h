#pragma once
#include "Vector2.h"

class WVector3
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
	WVector3();
	WVector3(float i);
	WVector3(const WVector3&v);
	WVector3(const WVector2&v);
	WVector3(float ix,float iy,float iz);

	//析构函数
	WVector3::~WVector3(void);
	//加法
	WVector3 operator+(const WVector3&i)const;
	WVector3 operator+=(const WVector3&i);

	//减法
	WVector3 operator-(const WVector3&i)const;
	WVector3 operator-=(const WVector3&i);

	//数乘
	friend WVector3 operator*(const WVector3&i,float n);
	friend WVector3 operator*(float n,const WVector3&i);
	friend WVector3 operator*(const WVector3&i1,const WVector3&i2);

	WVector3 operator*=(float f);
	WVector3 operator*=(const WVector3&i);

	//数除
	friend WVector3 operator/(const WVector3&i,float n);
	friend WVector3 operator/(float n,const WVector3&i);
	friend WVector3 operator/(const WVector3&i,const WVector3&n);

	WVector3 operator/=(float f);
	WVector3 operator=(const WVector3&i);

	bool operator==(const WVector3& i)const;

	//长度
	float length()const;
	float lengthSquared()const;
	//单位化
	WVector3 normalize();
	//计算反射向量
	WVector3 reflect(const WVector3 normal)const;
	WVector3 sqrtElement()const;

	//点乘
	float dot(const WVector3&i)const;
	//叉乘
	WVector3 cross(const WVector3&i)const;

	//输出值，调试时用
	void showCoords()const;
};
