#include "StdAfx.h"
#include "Vector4.h"


WVector4::WVector4(){x=y=z=0.0f;h=1.0f;}
WVector4::WVector4(float ix,float iy,float iz,float ih):x(ix),y(iy),z(iz),h(ih){}
WVector4::WVector4(float i):x(i),y(i),z(i),h(i){};
WVector4::WVector4(const WVector4&v)
{
	x=v.x;y=v.y;z=v.z;h=v.h;
}
WVector4::WVector4(const WVector3&v)
{
	x=v.x;y=v.y;z=v.z;h=1.0f;
}
WVector4::WVector4(const WVector2&v)
{
	x=v.x;y=v.y;z=0.0f;h=1.0f;
}
//析构函数
WVector4::~WVector4(void){}
//加法
WVector4 WVector4::operator +(const WVector4&i)
{
	return WVector4(x+i.x,y+i.y,z+i.z,h+i.h);
}
WVector4 WVector4::operator +=(const WVector4&i)
{
	x+=i.x;y+=i.y;z+=i.z;h+=i.h;return *this;
}
//减法
WVector4 WVector4::operator -(const WVector4&i)
{
	return WVector4(x-i.x,y-i.y,z-i.z,h-i.h);
}
WVector4 WVector4::operator -=(const WVector4&i)
{
	x-=i.x;y-=i.y;z-=i.z;h-=i.h;return *this;
}
//数乘
WVector4 operator*(const WVector4&i,float n)
{
	return WVector4(i.x*n,i.y*n,i.z*n,i.h*n);
}
WVector4 operator*(float n,const WVector4&i)
{
	return WVector4(i.x*n,i.y*n,i.z*n,i.h*n);
}
WVector4 WVector4::operator*=(float f)
{
	x*=f;y*=f;z*=f;h*=f;return *this;
}
//数除
WVector4 operator/(const WVector4&i,float n)
{
	return WVector4(i.x/n,i.y/n,i.z/n,i.h/n);
}
WVector4 operator/(float n,const WVector4&i)
{
	return WVector4(n/i.x,n/i.y,n/i.z,n/i.h);
}
WVector4 WVector4:: operator/=(float f)
{
	x/=f;y/=f;z/=f;h/=f;return *this;
}
//长度
float WVector4::length()
{
	return sqrt(x*x+y*y+z*z+h*h);
}
float WVector4::lengthSquared()
{
	return x*x+y*y+z*z+h*h;
}
//单位化
WVector4 WVector4::normalize()
{
	float invLength=1.0f/sqrt(x*x+y*y+z*z+h*h);
	x*=invLength;y*=invLength;z*=invLength;h*=invLength;
	return *this;
}
//点乘
float WVector4::dot(const WVector4&i)
{
	return x*i.x+y*i.y+z*i.z+h*i.h;
}