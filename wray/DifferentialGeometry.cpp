#include "stdafx.h"
#include "Vector2.h"
#include "Vector3.h"
#include <math.h>
#include "DifferentialGeometry.h"
#include <gl/glut.h>
WDifferentialGeometry::WDifferentialGeometry(void)
{
// 	position=WVector3(0.0f,0.0f,0.0f);
// 	normal=WVector3(0.0f,0.0f,1.0f);
// 	tangent=WVector3(1.0f,0.0f,0.0f);
// 	bitangent=WVector3(0.0f,1.0f,0.0f);
// 	rayDir=WVector3(1.0f,0.0f,1.0f);
// 	texCoord=Vector2(0.0f,0.0f);
// 	dpdu=dpdv=dndu=dndv=WVector3(0.0f,0.0f,0.0f);
// 	mtlId=0;
}
WDifferentialGeometry::WDifferentialGeometry(const WDifferentialGeometry&DG)
{
	*this=DG;
}
WDifferentialGeometry::WDifferentialGeometry(
				 const WVector3&iposition,const WVector3&inormal,
				 const WVector3&irayDir,const WVector2&itexCoord,
				 const WVector3&idpdu,const WVector3&idpdv,
				 const WVector3&idndu,const WVector2&idndv,
				 unsigned int imtlId):
position(iposition),normal(inormal),
rayDir(irayDir),
texCoord(itexCoord),dpdu(idpdu),dpdv(idpdv),
dndu(idndu),dndv(idndv),mtlId(imtlId)
{
	normal.normalize();
	bitangent=normal.cross(rayDir);
	if(bitangent.lengthSquared()<1e-3f)
	{
		//认为是零向量，此时normal和rayDir或者共线，或者至少一个为0
		if(abs(normal.x)>abs(normal.y))
		{
			bitangent=WVector3(-normal.z,0.0f,normal.x);
		}
		else
		{
			bitangent=WVector3(0.0f,normal.z,-normal.y);
		}
	}
	bitangent.normalize();
	tangent=bitangent.cross(normal);
}
WDifferentialGeometry::WDifferentialGeometry(
				 const WVector3&iposition,const WVector3&inormal,
				 const WVector3&itangent,const WVector3&ibitangent,
				 const WVector3&irayDir,const WVector2&itexCoord,
				 const WVector3&idpdu,const WVector3&idpdv,
				 const WVector3&idndu,const WVector2&idndv,
				 unsigned int imtlId
				 ):position(iposition),normal(inormal),
				 tangent(itangent),bitangent(ibitangent),
				 rayDir(irayDir),texCoord(itexCoord),
				 dpdu(idpdu),dpdv(idpdv),dndu(idndu),dndv(idndv),
				 mtlId(imtlId){}

WDifferentialGeometry::~WDifferentialGeometry(void){}
void WDifferentialGeometry::draw()
{
	glBegin(GL_POINTS);
	glVertex3f(position.x,position.y,position.z);
	glEnd();
	WVector3 normalEnd=position+0.5*normal;
	WVector3 tangentEnd=position+0.5*tangent;
	WVector3 bitangentEnd=position+0.5*bitangent;
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	glColor3f(0.0f,0.0f,1.0f);
	glVertex3f(position.x,position.y,position.z);
	glVertex3f(normalEnd.x,normalEnd.y,normalEnd.z);
	glColor3f(1.0f,0.0f,0.0f);
	glVertex3f(position.x,position.y,position.z);
	glVertex3f(tangentEnd.x,tangentEnd.y,tangentEnd.z);
	glColor3f(0.0f,1.0f,0.0f);
	glVertex3f(position.x,position.y,position.z);
	glVertex3f(bitangentEnd.x,bitangentEnd.y,bitangentEnd.z);
	glEnd();
	glColor3f(0.3f,0.3f,0.3f);
	glLineWidth(1.0f);
}