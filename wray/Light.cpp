#include "StdAfx.h"
#include "Light.h"



WLight::~WLight(void)
{
}

void WPointLight::sampleLight(float u1, float u2, float u3,
							 WBSDF&bsdf,WVector3 &iposition, 
							 WVector3 &iintensity, float&PDF)
{
	iposition=position;
	WVector3 delta=iposition-bsdf.DG.position;
	float distanceSquared=delta.lengthSquared();
	iintensity=intensity/distanceSquared;
	PDF=1.0f;
}
void WPointLight::draw()
{
	float delta=0.1f;
	WVector3 xn,xp,yn,yp,zn,zp;
	xn=xp=yn=yp=zn=zp=position;
	xp+=WVector3(delta,0,0);
	xn-=WVector3(delta,0,0);	
	yp+=WVector3(0,delta,0);
	yn-=WVector3(0,delta,0);	
	zp+=WVector3(0,0,delta);
	zn-=WVector3(0,0,delta);
	glColor3f(1.0,0.5,0);
	glLineWidth(5.0f);
	glBegin(GL_LINES);
	glVertex3f(xn.x,xn.y,xn.z);	glVertex3f(xp.x,xp.y,xp.z);
	glVertex3f(yn.x,yn.y,yn.z);	glVertex3f(yp.x,yp.y,yp.z);
	glVertex3f(zn.x,zn.y,zn.z);	glVertex3f(zp.x,zp.y,zp.z);
	glEnd();
	glLineWidth(1.0f);
}

void WPointLight::getProperties( vector<float>& properties )
{
	properties.clear();
	properties.push_back(intensity.x);
	properties.push_back(intensity.y);
	properties.push_back(intensity.z);
	properties.push_back(1.0f);
	properties.push_back(position.x);
	properties.push_back(position.y);
	properties.push_back(position.z);
	properties.push_back(1.0f);
}

 WRectangleLight::WRectangleLight(WVector3 iposition,WVector3 idirection,
			   WVector3 up,float width,float height,WVector3 iintensity,bool iisDoubleSide):
WLight(LIGHT_RECTANGLE,false)
{
	position=iposition;
	direction=idirection;
	direction.normalize();
	x=up.cross(direction);
	y=direction.cross(x);
	x.normalize();y.normalize();
	x*=width;y*=height;
	area=x.length()*y.length()*4;
	intensity=iintensity;
	isDoubleSide=iisDoubleSide;
}
void WRectangleLight::sampleLight(float u1, float u2, float u3, WBSDF &bsdf, WVector3 &iposition, WVector3 &iintensity, float &PDF)
{
	u1=u1*2-1;u2=u2*2-1;
	iposition=position+u1*x+u2*y;
	WVector3 delta=bsdf.DG.position-iposition;
	float distanceSquared=delta.lengthSquared();
	iintensity=intensity/distanceSquared;
	delta.normalize();
	float cosTheta=delta.dot(direction);
	if(isDoubleSide)
		cosTheta=abs(cosTheta);
	PDF=distanceSquared/(area*cosTheta);
}
void WRectangleLight::draw()
{
	WVector3 xpyp,xnyp,xpyn,xnyn;
	xpyp=position+x+y;
	xnyp=position-x+y;
	xpyn=position+x-y;
	xnyn=position-x-y;
	WVector3 end=position+direction;
	glColor3f(1.0,0.5,0);
	glBegin(GL_LINES);
	glVertex3f(xpyp.x,xpyp.y,xpyp.z);
	glVertex3f(xnyp.x,xnyp.y,xnyp.z);

	glVertex3f(xpyp.x,xpyp.y,xpyp.z);
	glVertex3f(xpyn.x,xpyn.y,xpyn.z);

	glVertex3f(xnyn.x,xnyn.y,xnyn.z);
	glVertex3f(xnyp.x,xnyp.y,xnyp.z);

	glVertex3f(xnyn.x,xnyn.y,xnyn.z);
	glVertex3f(xpyn.x,xpyn.y,xpyn.z);

	glVertex3f(position.x,position.y,position.z);
	glVertex3f(end.x,end.y,end.z);
	glEnd();
}

void WRectangleLight::getProperties( vector<float>& properties )
{
	properties.clear();	
	properties.push_back(position.x);
	properties.push_back(position.y);
	properties.push_back(position.z);
	properties.push_back(1.0f);

	properties.push_back(direction.x);
	properties.push_back(direction.y);
	properties.push_back(direction.z);
	properties.push_back(1.0f);

	properties.push_back(x.x);
	properties.push_back(x.y);
	properties.push_back(x.z);
	properties.push_back(1.0f);

	properties.push_back(y.x);
	properties.push_back(y.y);
	properties.push_back(y.z);
	properties.push_back(area);		//这是灯的面积

	properties.push_back(intensity.x);
	properties.push_back(intensity.y);
	properties.push_back(intensity.z);
	properties.push_back(1.0f);
}
/*
void ObjectLight::clear()
{
	faces.clear();
}
void ObjectLight::addPrimitive(unsigned int nthPrimitive)
{
	if(nthPrimitive>=scene.getPrimNum()||
		nthPrimitive<0)
		return;
	Primitive*prim;
	scene->getNthPrimitive(prim,nthPrimitive);
	unsigned int nFace;
	prim->getFaceNum(nFace);
	for(int i=0;i<nFace;i++)
	{
		Triangle tri;
		prim->getTriangle(i,tri);
		faces.push_back(tri);
	}
}
void ObjectLight::sampleLight(
							  float u1,float u2,float u3,
							  WBSDF&bsdf,
							  WVector3&iposition,
							  WVector3&iintensity,
							  float&PDF)
{
	iintensity=intensity;
	unsigned int nthTri=unsigned int(u1*faces.size());
	Triangle t=faces[nthTri];
	float b1,b2;
	MC::uniformSampleTriangle(u2,u3,b1,b2);
	WVector3 normal;
	Vector2 texCoord;
	t.getPoint(b1,b2,iposition,normal,texCoord);
	WVector3 delta=bsdf.DG.position-iposition;
	float distanceSquared=delta.lengthSquared();
	iintensity=intensity/distanceSquared;
	delta.normalize();
	float cosTheta=delta.dot(direction);
	if(isDoubleSide)
		cosTheta=abs(cosTheta);
	PDF=distanceSquared/(t.area*cosTheta);
}
*/
