#include "stdafx.h"
#include "Ray.h"
#include <gl/glut.h>
Ray::Ray(void)
{
// 	point.x=point.y=point.z=0.0f;
// 	direction.x=1.0f;
// 	direction.y=direction.z=0.0f;
	tMin=RAY_EPSILON;
	tMax=M_INF_BIG;
}
Ray::Ray(const Vector3&iPoint,const Vector3&iDirection)
{
	point=iPoint;
	direction=iDirection;
	tMin=RAY_EPSILON;
	tMax=M_INF_BIG;
}
Ray::Ray(const Vector3&iPoint,const Vector3&iDirection,
		 float iTMin,float iTMax)
{
	point=iPoint;
	direction=iDirection;
	tMin=iTMin;
	tMax=iTMax;
}
Ray::Ray(const Vector3&iDirection)
{
	point.x=point.y=point.z=0.0f;
	direction=iDirection;
	tMin=RAY_EPSILON;
	tMax=M_INF_BIG;
}
Ray::~Ray(void)
{
}
void Ray::normalizeDir()
{
	direction.normalize();
}
void Ray::inverseDir()
{
	direction*=-1.0f;
}
void Ray::reflect(Vector3 normal)
{
	normal.normalize();
	float dCos=direction.dot(normal);
	Vector3 delta=normal*dCos-direction;
	direction+=2.0f*delta;
}
void Ray::draw()
{
	Vector3 lineEnd=point+direction*100.0f;
	glBegin(GL_LINES);
	glVertex3f(point.x,point.y,point.z);
	glVertex3f(lineEnd.x,lineEnd.y,lineEnd.z);
	glEnd();
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	glVertex3f(point.x,point.y,point.z);
	glEnd();
}
void Ray::drawSegment()
{
	float t=tMax>100.0f?100.0f:tMax;
	Vector3 lineEnd=point+direction*t;
	glBegin(GL_LINES);
	glVertex3f(point.x,point.y,point.z);
	glVertex3f(lineEnd.x,lineEnd.y,lineEnd.z);
	glEnd();
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	glVertex3f(point.x,point.y,point.z);
	glEnd();
}