#include "BoundingBox.h"
#include <GL/glut.h>
#include "stdafx.h"
float WBoundingBox::delta = 1e-4f;
WBoundingBox::WBoundingBox(void)
{
}
WBoundingBox::WBoundingBox(const WTriangle& it, bool isBigger)
{
	float e;
	if(isBigger)
		e = delta;
	else
		e = 0.0f;
	pMin = it.point1 - WVector3(e);
	pMax = it.point1 + WVector3(e);
	merge(it.point2, isBigger);
	merge(it.point3, isBigger);
}
WBoundingBox::~WBoundingBox(void)
{
}
WBoundingBox::WBoundingBox(WVector3 ipMin,WVector3 ipMax)
{
	pMin=ipMin;pMax=ipMax;
}

WBoundingBox::WBoundingBox( float box[2][3] )
{
	pMin.x = box[0][0];
	pMin.y = box[0][1];
	pMin.z = box[0][2];
	pMax.x = box[1][0];
	pMax.y = box[1][1];
	pMax.z = box[1][2];
}
void WBoundingBox::merge(const WVector3&point,bool isBigger)
{
	float e = 0.0f;
	if(isBigger)
		e = delta;
	if(pMin.x>point.x-e)pMin.x=point.x-e;
	if(pMin.y>point.y-e)pMin.y=point.y-e;
	if(pMin.z>point.z-e)pMin.z=point.z-e;
	if(pMax.x<point.x+e)pMax.x=point.x+e;
	if(pMax.y<point.y+e)pMax.y=point.y+e;
	if(pMax.z<point.z+e)pMax.z=point.z+e;
}
void WBoundingBox::merge(const WBoundingBox&iBox)
{
	if(pMin.x>iBox.pMin.x)pMin.x=iBox.pMin.x;
	if(pMin.y>iBox.pMin.y)pMin.y=iBox.pMin.y;
	if(pMin.z>iBox.pMin.z)pMin.z=iBox.pMin.z;
	if(pMax.x<iBox.pMax.x)pMax.x=iBox.pMax.x;
	if(pMax.y<iBox.pMax.y)pMax.y=iBox.pMax.y;
	if(pMax.z<iBox.pMax.z)pMax.z=iBox.pMax.z;
}
void WBoundingBox::merge(const WTriangle&t,bool isBigger )
{
	merge(t.point1,isBigger);
	merge(t.point2,isBigger);
	merge(t.point3,isBigger);
}
void WBoundingBox::draw()const
{
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glDisable(GL_CULL_FACE);
	glBegin(GL_QUAD_STRIP);
	glVertex3f(pMin.x,pMin.y,pMin.z);
	glVertex3f(pMax.x,pMin.y,pMin.z);
	glVertex3f(pMin.x,pMax.y,pMin.z);
	glVertex3f(pMax.x,pMax.y,pMin.z);
	glVertex3f(pMin.x,pMax.y,pMax.z);
	glVertex3f(pMax.x,pMax.y,pMax.z);
	glVertex3f(pMin.x,pMin.y,pMax.z);
	glVertex3f(pMax.x,pMin.y,pMax.z);
	glVertex3f(pMin.x,pMin.y,pMin.z);
	glVertex3f(pMax.x,pMin.y,pMin.z);
	glEnd();
}
inline void WBoundingBox::swap(float&t1,float&t2)
{
	float temp=t1;t1=t2;t2=temp;
}
bool WBoundingBox::isIntersect(
							  const WRay&r,
							  float&tMin,
							  float&tMax)
{
	float t0,t1;
	t0 = (pMin.x - r.point.x) / r.direction.x;
	t1 = (pMax.x - r.point.x) / r.direction.x;
	tMin = min(t0,t1);
	tMax = max(t0,t1);
	t0 = (pMin.y - r.point.y) / r.direction.y;
	t1 = (pMax.y - r.point.y) / r.direction.y;
	tMin = max(tMin,min(t0,t1));
	tMax = min(tMax,max(t0,t1));		
	t0 = (pMin.z - r.point.z) / r.direction.z;
	t1 = (pMax.z - r.point.z) / r.direction.z;
	tMin = max(tMin,min(t0,t1));
	tMax = min(tMax,max(t0,t1));
	return !((tMin > tMax) | (r.tMin > tMax) | (r.tMax < tMin));
}
int WBoundingBox::maxAxis()
{
	float x=pMax.x-pMin.x;
	float y=pMax.y-pMin.y;
	float z=pMax.z-pMin.z;
	return (x>y)?((x>z)?0:2):((y>z)?1:2);
}
void WBoundingBox::displayCoords()const
{
	cout<<"pMin: \t";
	pMin.showCoords();
	cout<<"pMax: ";
	pMax.showCoords();
	cout<<endl;
}
WBoundingBox WBoundingBox::operator =(const WBoundingBox&box)
{
	pMax=box.pMax;
	pMin=box.pMin;
	return *this;
}
float WBoundingBox::halfArea()
{
	WVector3 boxExtent=pMax-pMin;
	return boxExtent.x*boxExtent.y+
			boxExtent.y*boxExtent.z+
			boxExtent.z*boxExtent.x;
}

float WBoundingBox::area()
{
	return halfArea()*2.0f;
}

bool WBoundingBox::operator==( const WBoundingBox& box ) const
{
	if(pMin == box.pMin && pMax == box.pMax)
		return true;
	return false;
}

void WBoundingBox::getTightBound( WVector3& tightMin, WVector3& tightMax )
{
	tightMin = pMin + WVector3(delta);
	tightMax = pMax - WVector3(delta);
}

bool WBoundingBox::intersect( const WBoundingBox& iBox, WBoundingBox& resultBox )
{
	resultBox.pMin.x = pMin.x > iBox.pMin.x ? pMin.x : iBox.pMin.x;
	resultBox.pMax.x = pMax.x < iBox.pMax.x ? pMax.x : iBox.pMax.x;
	if (resultBox.pMin.x > resultBox.pMax.x)
		return false;
	resultBox.pMin.y = pMin.y > iBox.pMin.y ? pMin.y : iBox.pMin.y;
	resultBox.pMax.y = pMax.y < iBox.pMax.y ? pMax.y : iBox.pMax.y;
	if (resultBox.pMin.y > resultBox.pMax.y)
		return false;
	resultBox.pMin.z = pMin.z > iBox.pMin.z ? pMin.z : iBox.pMin.z;
	resultBox.pMax.z = pMax.z < iBox.pMax.z ? pMax.z : iBox.pMax.z;
	if (resultBox.pMin.z > resultBox.pMax.z)
		return false;
	return true;
}

bool WBoundingBox::isInBoxInclusive( const WVector3& point )
{
	return point.x >= pMin.x && point.y >= pMin.y && point.z >= pMin.z &&
		   point.x <= pMax.x && point.y <= pMax.y && point.z <= pMax.z ;
}

bool WBoundingBox::isInBoxExclusive( const WVector3& point )
{
	return point.x > pMin.x && point.y > pMin.y && point.z > pMin.z &&
		   point.x < pMax.x && point.y < pMax.y && point.z < pMax.z ;
}

bool WBoundingBox::onFace( WVector3& point, int& face )
{
	if (point.x == pMin.x)
	{
		face = 1; return true;
	} 
	else if (point.x == pMax.x)
	{
		face = 0; return true;
	}
	else if (point.y == pMin.y)
	{
		face = 3; return true;
	} 
	else if (point.y == pMax.y)
	{
		face = 2; return true;
	}
	else if (point.z == pMin.z)
	{
		face = 5; return true;
	} 
	else if (point.z == pMax.z)
	{
		face = 4; return true;
	}
	return false;
}