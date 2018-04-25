#include "StdAfx.h"
#include "Vector4.h"


Vector4::Vector4(){x=y=z=0.0f;h=1.0f;}
Vector4::Vector4(float ix,float iy,float iz,float ih):x(ix),y(iy),z(iz),h(ih){}
Vector4::Vector4(float i):x(i),y(i),z(i),h(i){};
Vector4::Vector4(const Vector4&v)
{
	x=v.x;y=v.y;z=v.z;h=v.h;
}
Vector4::Vector4(const Vector3&v)
{
	x=v.x;y=v.y;z=v.z;h=1.0f;
}
Vector4::Vector4(const Vector2f&v)
{
	x=v.x;y=v.y;z=0.0f;h=1.0f;
}
//析构函数
Vector4::~Vector4(void){}
//加法
Vector4 Vector4::operator +(const Vector4&i)
{
	return Vector4(x+i.x,y+i.y,z+i.z,h+i.h);
}
Vector4 Vector4::operator +=(const Vector4&i)
{
	x+=i.x;y+=i.y;z+=i.z;h+=i.h;return *this;
}
//减法
Vector4 Vector4::operator -(const Vector4&i)
{
	return Vector4(x-i.x,y-i.y,z-i.z,h-i.h);
}
Vector4 Vector4::operator -=(const Vector4&i)
{
	x-=i.x;y-=i.y;z-=i.z;h-=i.h;return *this;
}
//数乘
Vector4 operator*(const Vector4&i,float n)
{
	return Vector4(i.x*n,i.y*n,i.z*n,i.h*n);
}
Vector4 operator*(float n,const Vector4&i)
{
	return Vector4(i.x*n,i.y*n,i.z*n,i.h*n);
}
Vector4 Vector4::operator*=(float f)
{
	x*=f;y*=f;z*=f;h*=f;return *this;
}
//数除
Vector4 operator/(const Vector4&i,float n)
{
	return Vector4(i.x/n,i.y/n,i.z/n,i.h/n);
}
Vector4 operator/(float n,const Vector4&i)
{
	return Vector4(n/i.x,n/i.y,n/i.z,n/i.h);
}
Vector4 Vector4:: operator/=(float f)
{
	x/=f;y/=f;z/=f;h/=f;return *this;
}
//长度
float Vector4::length()
{
	return sqrt(x*x+y*y+z*z+h*h);
}
float Vector4::lengthSquared()
{
	return x*x+y*y+z*z+h*h;
}
//单位化
Vector4 Vector4::normalize()
{
	float invLength=1.0f/sqrt(x*x+y*y+z*z+h*h);
	x*=invLength;y*=invLength;z*=invLength;h*=invLength;
	return *this;
}
//点乘
float Vector4::dot(const Vector4&i)
{
	return x*i.x+y*i.y+z*i.z+h*i.h;
}