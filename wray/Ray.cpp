#include "stdafx.h"
#include "Ray.h"
#include <gl/glut.h>
WRay::WRay(void)
{
// 	point.x=point.y=point.z=0.0f;
// 	direction.x=1.0f;
// 	direction.y=direction.z=0.0f;
	tMin=RAY_EPSILON;
	tMax=M_INF_BIG;
}
WRay::WRay(const WVector3&iPoint,const WVector3&iDirection)
{
	point=iPoint;
	direction=iDirection;
	tMin=RAY_EPSILON;
	tMax=M_INF_BIG;
}
WRay::WRay(const WVector3&iPoint,const WVector3&iDirection,
		 float iTMin,float iTMax)
{
	point=iPoint;
	direction=iDirection;
	tMin=iTMin;
	tMax=iTMax;
}
WRay::WRay(const WVector3&iDirection)
{
	point.x=point.y=point.z=0.0f;
	direction=iDirection;
	tMin=RAY_EPSILON;
	tMax=M_INF_BIG;
}
WRay::~WRay(void)
{
}
void WRay::normalizeDir()
{
	direction.normalize();
}
void WRay::inverseDir()
{
	direction*=-1.0f;
}
void WRay::reflect(WVector3 normal)
{
	normal.normalize();
	float dCos=direction.dot(normal);
	WVector3 delta=normal*dCos-direction;
	direction+=2.0f*delta;
}
void WRay::draw()
{
	WVector3 lineEnd=point+direction*100.0f;
	glBegin(GL_LINES);
	glVertex3f(point.x,point.y,point.z);
	glVertex3f(lineEnd.x,lineEnd.y,lineEnd.z);
	glEnd();
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	glVertex3f(point.x,point.y,point.z);
	glEnd();
}
void WRay::drawSegment()
{
	float t=tMax>100.0f?100.0f:tMax;
	WVector3 lineEnd=point+direction*t;
	glBegin(GL_LINES);
	glVertex3f(point.x,point.y,point.z);
	glVertex3f(lineEnd.x,lineEnd.y,lineEnd.z);
	glEnd();
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	glVertex3f(point.x,point.y,point.z);
	glEnd();
}