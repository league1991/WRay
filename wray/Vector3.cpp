#include "StdAfx.h"
#include "Vector3.h"
//构造函数
Vector3::Vector3(){x=y=z=0.0f;}
Vector3::Vector3(float ix,float iy,float iz):x(ix),y(iy),z(iz){}
Vector3::Vector3(float i):x(i),y(i),z(i){};
Vector3::Vector3(const Vector3&v)
{
	x=v.x;y=v.y;z=v.z;
}
Vector3::Vector3(const Vector2&v)
{
	x=v.x;y=v.y;z=0.0f;
}
//析构函数
Vector3::~Vector3(void){}
//加法
Vector3 Vector3::operator +(const Vector3&i)const
{
	return Vector3(x+i.x,y+i.y,z+i.z);
}
Vector3 Vector3::operator +=(const Vector3&i)
{
	x+=i.x;y+=i.y;z+=i.z;return *this;
}
//减法
Vector3 Vector3::operator -(const Vector3&i)const
{
	return Vector3(x-i.x,y-i.y,z-i.z);
}
Vector3 Vector3::operator -=(const Vector3&i)
{
	x-=i.x;y-=i.y;z-=i.z;return *this;
}
//数乘
Vector3 operator*(const Vector3&i,float n)
{
	return Vector3(i.x*n,i.y*n,i.z*n);
}
Vector3 operator*(float n,const Vector3&i)
{
	return Vector3(i.x*n,i.y*n,i.z*n);
}
Vector3 operator*(const Vector3&i1,const Vector3&i2)
{
//	cout<<1<<endl;
	return Vector3(i1.x*i2.x,i1.y*i2.y,i1.z*i2.z);
}
Vector3 Vector3::operator*=(float f)
{
	x*=f;y*=f;z*=f;return *this;
}
Vector3 Vector3::operator*=(const Vector3&i)
{
	this->x*=i.x;this->y*=i.y;this->z*=i.z;
	return *this;
}
//数除
Vector3 operator/(const Vector3&i,float n)
{
	return Vector3(i.x/n,i.y/n,i.z/n);
}
Vector3 operator/(const Vector3&i,const Vector3&n)
{
	return Vector3(i.x/n.x,i.y/n.y,i.z/n.z);
}
Vector3 operator/(float n,const Vector3&i)
{
	return Vector3(n/i.x,n/i.y,n/i.z);
}
Vector3 Vector3:: operator/=(float f)
{
	x/=f;y/=f;z/=f;return *this;
}
//长度
float Vector3::length()const
{
	return sqrt(x*x+y*y+z*z);
}
float Vector3::lengthSquared()const
{
	return x*x+y*y+z*z;
}
//单位化
Vector3 Vector3::normalize()
{
	float invLength=1.0f/sqrt(x*x+y*y+z*z);
	x*=invLength;y*=invLength;z*=invLength;
	return *this;
}
//点乘
float Vector3::dot(const Vector3&i)const
{
	return x*i.x+y*i.y+z*i.z;
}
//叉乘
Vector3 Vector3::cross(const Vector3&i)const
{
	return Vector3(
		y*i.z-z*i.y,
		z*i.x-x*i.z,
		x*i.y-y*i.x);
}
Vector3 Vector3::reflect(Vector3 normal)const
{
//	normal.normalize();
// 	float dCos=this->dot(normal);
// 	Vector3 delta=normal*dCos-(*this);
// 	return (*this)+2.0f*delta;
	Vector3 r=(*this)*(-1.0f);
	return r+normal*this->dot(normal)*2.0f;
}
void Vector3::showCoords()const
{
	printf("%.3f %.3f %.3f\n", x, y, z);
//	cout<<x<<' '<<y<<' '<<z<<endl;
}
Vector3 Vector3::operator =(const Vector3&i)
{
	x=i.x;y=i.y;z=i.z;
	return *this;
}
Vector3 Vector3::sqrtElement()const
{
	return Vector3(sqrt(x),sqrt(y),sqrt(z));
}

bool Vector3::operator==( const Vector3& i ) const
{
	if(x == i.x && y == i.y && z == i.z)
		return true;
	return false;
}