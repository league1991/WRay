#include "StdAfx.h"
#include "Camera.h"

WCamera::WCamera(void)
{
	origin=WVector3(0,0,1);
	target=WVector3(0,0,0);
	up=WVector3(0,1,0);
	fov=M_2_PI*0.5;
	ratio=1.33f;
	filmSampleSize=1;
	computeXY();
}

WCamera::~WCamera(void)
{
}
void WCamera::setParameter(WVector3 iori, WVector3 itar, WVector3 iup, float ifov,float iratio)
{
	origin=iori;
	target=itar;
	up=iup;
	fov=ifov;
	ratio=iratio;
	computeXY();
}
void WCamera::setDirectionParams(WVector3 iori, WVector3 itar, WVector3 iup)
{
	origin=iori;
	target=itar;
	up=iup;
	computeXY();
}
void WCamera::computeXY()
{
	dir=target-origin;
	dir.normalize();
	x=dir.cross(up);
	x.normalize();
	y=x.cross(dir);
	y.normalize();
	float Ly=tan(fov/2.0f);
	y*=Ly;
	x*=(Ly*ratio);
}
void WCamera::drawCamera(float R,float G,float B)
{
	//屏幕的四个角的空间位置
	WVector3 xpyp,xnyp,xpyn,xnyn;
	WVector3 center=origin+dir;
	xpyp=center+x+y;
	xpyn=center+x-y;
	xnyp=center-x+y;
	xnyn=center-x-y;
	glColor3f(R,G,B);
	glBegin(GL_LINES);

	glVertex3f(origin.x,origin.y,origin.z);
	glVertex3f(xpyp.x,xpyp.y,xpyp.z);
	glVertex3f(origin.x,origin.y,origin.z);
	glVertex3f(xpyn.x,xpyn.y,xpyn.z);
	glVertex3f(origin.x,origin.y,origin.z);
	glVertex3f(xnyp.x,xnyp.y,xnyp.z);
	glVertex3f(origin.x,origin.y,origin.z);
	glVertex3f(xnyn.x,xnyn.y,xnyn.z);

	glVertex3f(xpyp.x,xpyp.y,xpyp.z);
	glVertex3f(xpyn.x,xpyn.y,xpyn.z);

	glVertex3f(xpyn.x,xpyn.y,xpyn.z);
	glVertex3f(xnyn.x,xnyn.y,xnyn.z);

	glVertex3f(xnyn.x,xnyn.y,xnyn.z);
	glVertex3f(xnyp.x,xnyp.y,xnyp.z);

	glVertex3f(xnyp.x,xnyp.y,xnyp.z);
	glVertex3f(xpyp.x,xpyp.y,xpyp.z);

	glVertex3f(origin.x,origin.y,origin.z);
	glVertex3f(target.x,target.y,target.z);
	
	glEnd();
}
void WCamera::generateRay(float xRatio, float yRatio,WRay&ray)
{
	ray.point=origin;
	ray.direction=dir+x*xRatio+y*yRatio;
	ray.tMin=1e-5f;
	ray.tMax=M_INF_BIG;
}
void WCamera::drawFilmInScreen(float offsetX,float offsetY)
{
	WVector3 FilmPos=origin+dir-x*(1.0f+offsetX)-y*(1.0f+offsetY);
	film.draw(FilmPos.x,FilmPos.y,FilmPos.z);
}
void WCamera::drawFilmInWorld(float offsetX,float offsetY,float offsetZ)
{
	film.draw(offsetX,offsetY,offsetZ);
}
void WCamera::setFilmResolutionX(unsigned int resX,unsigned int resY)
{
	film.setResolution(resX,resY);
}
void WCamera::setFilmResolutionXY(unsigned int resX,unsigned int resY)
{
	film.setResolution(resX,resY);
	film.cleanColors(0,0,0);
	ratio=(float)resX/(float)resY;
}
void WCamera::cleanFilmColors(float R,float G,float B)
{
	film.cleanColors(R,G,B);
}
void WCamera::setFilmResolutionX(unsigned int resX)
{
	setFilmResolutionX(resX,(unsigned int)ceil(resX/ratio));
}
void WCamera::getFilmResolution(int&resX, int&resY)
{
	WVector2 res=film.getResolution();
	resX=(int)res.x;
	resY=(int)res.y;
}
void WCamera::getNextRay(WRay&ray)
{
	float screenX,screenY;
	film.getSamplePosition(screenX,screenY);
	WVector2 res=film.getResolution();
	screenX=screenX/res.x*2.0f-1.0f;
	screenY=screenY/res.y*2.0f-1.0f;
	ray.point=origin;
	ray.direction=dir+screenX*x+screenY*y;
	ray.direction.normalize();
	ray.tMin=1e-5f;
	ray.tMax=M_INF_BIG;
}

void WCamera::getNextRay( WRay&ray, float xi, float yi )
{
	WVector2 res=film.getResolution();
	float screenX= float(xi) /res.x*2.0f-1.0f;
	float screenY= float(yi) /res.y*2.0f-1.0f;
	ray.point=origin;
	ray.direction=dir+screenX*x+screenY*y;
	ray.direction.normalize();
	ray.tMin=1e-5f;
	ray.tMax=M_INF_BIG;
}

bool WCamera::isFilmFull()
{
	return film.isFull();
}
void WCamera::setColor(float R,float G,float B)
{
	film.setSampleColor(R,G,B);
}

void WCamera::setColor( float R, float G, float B, int x, int y )
{
	film.setColor(x, y, R, G, B);
}
void WCamera::accumulateColor( float R, float G, float B, int x, int y )
{
	film.accumulateColor(x, y, R, G, B);
}
void WCamera::clearFilm(float R,float G,float B)
{
	film.setCurrPos(0,0);
	film.cleanColors(R,G,B);
}
void WCamera::changeSampleSize(unsigned int size)
{
	film.changeSampleSize(size);
	filmSampleSize=size;
}
void WCamera::changeSampler(WSampler::WSamplerType type)
{
	film.changeSampler(type);
}
float WCamera::currProgress()
{
	unsigned int currX,currY;
	film.getCurrPos(currX,currY);
	WVector2 res=film.getResolution();
	float nPoints=currY*res.x+currX;
	float totalPoints=res.x*res.y;
	return nPoints/totalPoints;
}
int*WCamera::getFilmBitPointer()
{
	return film.getBitPointer();
}

WVector3 WCamera::getColor( unsigned int x,unsigned int y )
{
	return film.getColor(x,y);
}

void WCamera::nextExposurePass()
{
	film.nextExposurePass();
}