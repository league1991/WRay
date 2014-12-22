#include "stdafx.h"
#include "Vector2.h"
//构造函数
WVector2::WVector2(){x=y=0.0f;}
WVector2::WVector2(float ix,float iy):x(ix),y(iy){}
WVector2::WVector2(float i):x(i),y(i){}
WVector2::WVector2(const WVector2&v)
{
	x=v.x;y=v.y;
}
//加法
WVector2 WVector2::operator+(const WVector2&i)
{
	return WVector2(x+i.x,y+i.y);
}
WVector2 WVector2::operator+=(const WVector2&i)
{
	x+=i.x;y+=i.y;return *this;
}
//减法
WVector2 WVector2::operator-(const WVector2&i)
{
	return WVector2(x-i.x,y-i.y);
}
WVector2 WVector2::operator-=(const WVector2&i)
{
	x-=i.x;y-=i.y;return *this;
}
//数乘
WVector2 operator*(const WVector2&i1,float n)
{
	return WVector2(i1.x*n,i1.y*n);
}

WVector2 operator*(float n,const WVector2&i1)
{
	return WVector2(i1.x*n,i1.y*n);
}
WVector2 WVector2::operator*=(float f)
{ 
	x*=f;y*=f;return *this;
}
//数除
WVector2 operator/(const WVector2&i1,float n)
{
	float invn=1.0f/n;
	return WVector2(i1.x*invn,i1.y*invn);
}
WVector2 operator/(float n,const WVector2&i1)
{
	return WVector2(n/i1.x,n/i1.y);
}
WVector2 WVector2::operator/=(float f)
{
	x/=f;y/=f;return *this;
}
//长度
float WVector2::length()
{
	return sqrt(x*x+y*y);
}
float WVector2::lengthSquared()
{
	return x*x+y*y;
}
//单位化
WVector2 WVector2::normalize()
{
	float invLength=1.0f/sqrt(x*x+y*y);
	x*=invLength;y*=invLength;
	return *this;
}
//点乘
float WVector2::dot(const WVector2&i)
{
	return x*i.x+y*i.y;
}
void WVector2::showCoords()
{
	cout<<x<<' '<<y<<endl;
}