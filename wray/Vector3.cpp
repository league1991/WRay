#include "StdAfx.h"
#include "Vector3.h"
//构造函数
WVector3::WVector3(){x=y=z=0.0f;}
WVector3::WVector3(float ix,float iy,float iz):x(ix),y(iy),z(iz){}
WVector3::WVector3(float i):x(i),y(i),z(i){};
WVector3::WVector3(const WVector3&v)
{
	x=v.x;y=v.y;z=v.z;
}
WVector3::WVector3(const WVector2&v)
{
	x=v.x;y=v.y;z=0.0f;
}
//析构函数
WVector3::~WVector3(void){}
//加法
WVector3 WVector3::operator +(const WVector3&i)const
{
	return WVector3(x+i.x,y+i.y,z+i.z);
}
WVector3 WVector3::operator +=(const WVector3&i)
{
	x+=i.x;y+=i.y;z+=i.z;return *this;
}
//减法
WVector3 WVector3::operator -(const WVector3&i)const
{
	return WVector3(x-i.x,y-i.y,z-i.z);
}
WVector3 WVector3::operator -=(const WVector3&i)
{
	x-=i.x;y-=i.y;z-=i.z;return *this;
}
//数乘
WVector3 operator*(const WVector3&i,float n)
{
	return WVector3(i.x*n,i.y*n,i.z*n);
}
WVector3 operator*(float n,const WVector3&i)
{
	return WVector3(i.x*n,i.y*n,i.z*n);
}
WVector3 operator*(const WVector3&i1,const WVector3&i2)
{
//	cout<<1<<endl;
	return WVector3(i1.x*i2.x,i1.y*i2.y,i1.z*i2.z);
}
WVector3 WVector3::operator*=(float f)
{
	x*=f;y*=f;z*=f;return *this;
}
WVector3 WVector3::operator*=(const WVector3&i)
{
	this->x*=i.x;this->y*=i.y;this->z*=i.z;
	return *this;
}
//数除
WVector3 operator/(const WVector3&i,float n)
{
	return WVector3(i.x/n,i.y/n,i.z/n);
}
WVector3 operator/(const WVector3&i,const WVector3&n)
{
	return WVector3(i.x/n.x,i.y/n.y,i.z/n.z);
}
WVector3 operator/(float n,const WVector3&i)
{
	return WVector3(n/i.x,n/i.y,n/i.z);
}
WVector3 WVector3:: operator/=(float f)
{
	x/=f;y/=f;z/=f;return *this;
}
//长度
float WVector3::length()const
{
	return sqrt(x*x+y*y+z*z);
}
float WVector3::lengthSquared()const
{
	return x*x+y*y+z*z;
}
//单位化
WVector3 WVector3::normalize()
{
	float invLength=1.0f/sqrt(x*x+y*y+z*z);
	x*=invLength;y*=invLength;z*=invLength;
	return *this;
}
//点乘
float WVector3::dot(const WVector3&i)const
{
	return x*i.x+y*i.y+z*i.z;
}
//叉乘
WVector3 WVector3::cross(const WVector3&i)const
{
	return WVector3(
		y*i.z-z*i.y,
		z*i.x-x*i.z,
		x*i.y-y*i.x);
}
WVector3 WVector3::reflect(WVector3 normal)const
{
//	normal.normalize();
// 	float dCos=this->dot(normal);
// 	WVector3 delta=normal*dCos-(*this);
// 	return (*this)+2.0f*delta;
	WVector3 r=(*this)*(-1.0f);
	return r+normal*this->dot(normal)*2.0f;
}
void WVector3::showCoords()const
{
	printf("%.3f %.3f %.3f\n", x, y, z);
//	cout<<x<<' '<<y<<' '<<z<<endl;
}
WVector3 WVector3::operator =(const WVector3&i)
{
	x=i.x;y=i.y;z=i.z;
	return *this;
}
WVector3 WVector3::sqrtElement()const
{
	return WVector3(sqrt(x),sqrt(y),sqrt(z));
}

bool WVector3::operator==( const WVector3& i ) const
{
	if(x == i.x && y == i.y && z == i.z)
		return true;
	return false;
}