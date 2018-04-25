#include "stdafx.h"
#include "Vector2.h"
//构造函数
Vector2f::Vector2f(){x=y=0.0f;}
Vector2f::Vector2f(float ix,float iy):x(ix),y(iy){}
Vector2f::Vector2f(float i):x(i),y(i){}
Vector2f::Vector2f(const Vector2f&v)
{
	x=v.x;y=v.y;
}
//加法
Vector2f Vector2f::operator+(const Vector2f&i)
{
	return Vector2f(x+i.x,y+i.y);
}
Vector2f Vector2f::operator+=(const Vector2f&i)
{
	x+=i.x;y+=i.y;return *this;
}
//减法
Vector2f Vector2f::operator-(const Vector2f&i)
{
	return Vector2f(x-i.x,y-i.y);
}
Vector2f Vector2f::operator-=(const Vector2f&i)
{
	x-=i.x;y-=i.y;return *this;
}
//数乘
Vector2f operator*(const Vector2f&i1,float n)
{
	return Vector2f(i1.x*n,i1.y*n);
}

Vector2f operator*(float n,const Vector2f&i1)
{
	return Vector2f(i1.x*n,i1.y*n);
}
Vector2f Vector2f::operator*=(float f)
{ 
	x*=f;y*=f;return *this;
}
//数除
Vector2f operator/(const Vector2f&i1,float n)
{
	float invn=1.0f/n;
	return Vector2f(i1.x*invn,i1.y*invn);
}
Vector2f operator/(float n,const Vector2f&i1)
{
	return Vector2f(n/i1.x,n/i1.y);
}
Vector2f Vector2f::operator/=(float f)
{
	x/=f;y/=f;return *this;
}
//长度
float Vector2f::length()
{
	return sqrt(x*x+y*y);
}
float Vector2f::lengthSquared()
{
	return x*x+y*y;
}
//单位化
Vector2f Vector2f::normalize()
{
	float invLength=1.0f/sqrt(x*x+y*y);
	x*=invLength;y*=invLength;
	return *this;
}
//点乘
float Vector2f::dot(const Vector2f&i)
{
	return x*i.x+y*i.y;
}
void Vector2f::showCoords()
{
	cout<<x<<' '<<y<<endl;
}