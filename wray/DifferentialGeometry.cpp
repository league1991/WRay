#include "stdafx.h"
#include "Vector2.h"
#include "Vector3.h"
#include <math.h>
#include "DifferentialGeometry.h"
#include <gl/glut.h>
DifferentialGeometry::DifferentialGeometry(void)
{
// 	position=Vector3(0.0f,0.0f,0.0f);
// 	normal=Vector3(0.0f,0.0f,1.0f);
// 	tangent=Vector3(1.0f,0.0f,0.0f);
// 	bitangent=Vector3(0.0f,1.0f,0.0f);
// 	rayDir=Vector3(1.0f,0.0f,1.0f);
// 	texCoord=Vector2f(0.0f,0.0f);
// 	dpdu=dpdv=dndu=dndv=Vector3(0.0f,0.0f,0.0f);
// 	mtlId=0;
}
DifferentialGeometry::DifferentialGeometry(const DifferentialGeometry&DG)
{
	*this=DG;
}
DifferentialGeometry::DifferentialGeometry(
				 const Vector3&iposition,const Vector3&inormal,
				 const Vector3&irayDir,const Vector2f&itexCoord,
				 const Vector3&idpdu,const Vector3&idpdv,
				 const Vector3&idndu,const Vector2f&idndv,
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
			bitangent=Vector3(-normal.z,0.0f,normal.x);
		}
		else
		{
			bitangent=Vector3(0.0f,normal.z,-normal.y);
		}
	}
	bitangent.normalize();
	tangent=bitangent.cross(normal);
}
DifferentialGeometry::DifferentialGeometry(
				 const Vector3&iposition,const Vector3&inormal,
				 const Vector3&itangent,const Vector3&ibitangent,
				 const Vector3&irayDir,const Vector2f&itexCoord,
				 const Vector3&idpdu,const Vector3&idpdv,
				 const Vector3&idndu,const Vector2f&idndv,
				 unsigned int imtlId
				 ):position(iposition),normal(inormal),
				 tangent(itangent),bitangent(ibitangent),
				 rayDir(irayDir),texCoord(itexCoord),
				 dpdu(idpdu),dpdv(idpdv),dndu(idndu),dndv(idndv),
				 mtlId(imtlId){}

DifferentialGeometry::~DifferentialGeometry(void){}
void DifferentialGeometry::draw()
{
	glBegin(GL_POINTS);
	glVertex3f(position.x,position.y,position.z);
	glEnd();
	Vector3 normalEnd=position+0.5*normal;
	Vector3 tangentEnd=position+0.5*tangent;
	Vector3 bitangentEnd=position+0.5*bitangent;
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