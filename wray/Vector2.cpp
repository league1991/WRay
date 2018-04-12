#include "stdafx.h"
#include "Vector2.h"
//构造函数
Vector2::Vector2(){x=y=0.0f;}
Vector2::Vector2(float ix,float iy):x(ix),y(iy){}
Vector2::Vector2(float i):x(i),y(i){}
Vector2::Vector2(const Vector2&v)
{
	x=v.x;y=v.y;
}
//加法
Vector2 Vector2::operator+(const Vector2&i)
{
	return Vector2(x+i.x,y+i.y);
}
Vector2 Vector2::operator+=(const Vector2&i)
{
	x+=i.x;y+=i.y;return *this;
}
//减法
Vector2 Vector2::operator-(const Vector2&i)
{
	return Vector2(x-i.x,y-i.y);
}
Vector2 Vector2::operator-=(const Vector2&i)
{
	x-=i.x;y-=i.y;return *this;
}
//数乘
Vector2 operator*(const Vector2&i1,float n)
{
	return Vector2(i1.x*n,i1.y*n);
}

Vector2 operator*(float n,const Vector2&i1)
{
	return Vector2(i1.x*n,i1.y*n);
}
Vector2 Vector2::operator*=(float f)
{ 
	x*=f;y*=f;return *this;
}
//数除
Vector2 operator/(const Vector2&i1,float n)
{
	float invn=1.0f/n;
	return Vector2(i1.x*invn,i1.y*invn);
}
Vector2 operator/(float n,const Vector2&i1)
{
	return Vector2(n/i1.x,n/i1.y);
}
Vector2 Vector2::operator/=(float f)
{
	x/=f;y/=f;return *this;
}
//长度
float Vector2::length()
{
	return sqrt(x*x+y*y);
}
float Vector2::lengthSquared()
{
	return x*x+y*y;
}
//单位化
Vector2 Vector2::normalize()
{
	float invLength=1.0f/sqrt(x*x+y*y);
	x*=invLength;y*=invLength;
	return *this;
}
//点乘
float Vector2::dot(const Vector2&i)
{
	return x*i.x+y*i.y;
}
void Vector2::showCoords()
{
	cout<<x<<' '<<y<<endl;
}