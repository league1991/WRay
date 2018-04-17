#include "StdAfx.h"
#include "Camera.h"

Camera::Camera(void)
{
	origin=Vector3(0,0,1);
	target=Vector3(0,0,0);
	up=Vector3(0,1,0);
	fov=M_2_PI*0.5;
	ratio=1.33f;
	filmSampleSize=1;
	computeXY();
}

Camera::~Camera(void)
{
}
void Camera::setParameter(Vector3 iori, Vector3 itar, Vector3 iup, float ifov,float iratio)
{
	origin=iori;
	target=itar;
	up=iup;
	fov=ifov;
	ratio=iratio;
	computeXY();
}
void Camera::setDirectionParams(Vector3 iori, Vector3 itar, Vector3 iup)
{
	origin=iori;
	target=itar;
	up=iup;
	computeXY();
}
void Camera::computeXY()
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
void Camera::drawCamera(float R,float G,float B)
{
	//屏幕的四个角的空间位置
	Vector3 xpyp,xnyp,xpyn,xnyn;
	Vector3 center=origin+dir;
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
void Camera::generateRay(float xRatio, float yRatio,WRay&ray)
{
	ray.point=origin;
	ray.direction=dir+x*xRatio+y*yRatio;
	ray.tMin=1e-5f;
	ray.tMax=M_INF_BIG;
}
void Camera::drawFilmInScreen(float offsetX,float offsetY)
{
	Vector3 FilmPos=origin+dir-x*(1.0f+offsetX)-y*(1.0f+offsetY);
	film.draw(FilmPos.x,FilmPos.y,FilmPos.z);
}
void Camera::drawFilmInWorld(float offsetX,float offsetY,float offsetZ)
{
	film.draw(offsetX,offsetY,offsetZ);
}
void Camera::setFilmResolutionX(unsigned int resX,unsigned int resY)
{
	film.setResolution(resX,resY);
}
void Camera::setFilmResolutionXY(unsigned int resX,unsigned int resY)
{
	film.setResolution(resX,resY);
	film.cleanColors(0,0,0);
	ratio=(float)resX/(float)resY;
}
void Camera::cleanFilmColors(float R,float G,float B)
{
	film.cleanColors(R,G,B);
}
void Camera::setFilmResolutionX(unsigned int resX)
{
	setFilmResolutionX(resX,(unsigned int)ceil(resX/ratio));
}
void Camera::getFilmResolution(int&resX, int&resY)
{
	Vector2 res=film.getResolution();
	resX=(int)res.x;
	resY=(int)res.y;
}
void Camera::getNextRay(WRay&ray)
{
	float screenX,screenY;
	film.getSamplePosition(screenX,screenY);
	Vector2 res=film.getResolution();
	screenX=screenX/res.x*2.0f-1.0f;
	screenY=screenY/res.y*2.0f-1.0f;
	ray.point=origin;
	ray.direction=dir+screenX*x+screenY*y;
	ray.direction.normalize();
	ray.tMin=1e-5f;
	ray.tMax=M_INF_BIG;
}

void Camera::getNextRay( WRay&ray, float xi, float yi )
{
	Vector2 res=film.getResolution();
	float screenX= float(xi) /res.x*2.0f-1.0f;
	float screenY= float(yi) /res.y*2.0f-1.0f;
	ray.point=origin;
	ray.direction=dir+screenX*x+screenY*y;
	ray.direction.normalize();
	ray.tMin=1e-5f;
	ray.tMax=M_INF_BIG;
}

bool Camera::isFilmFull()
{
	return film.isFull();
}
void Camera::setColor(float R,float G,float B)
{
	film.setSampleColor(R,G,B);
}

void Camera::setColor( float R, float G, float B, int x, int y )
{
	film.setColor(x, y, R, G, B);
}
void Camera::accumulateColor( float R, float G, float B, int x, int y )
{
	film.accumulateColor(x, y, R, G, B);
}
void Camera::clearFilm(float R,float G,float B)
{
	film.setCurrPos(0,0);
	film.cleanColors(R,G,B);
}
void Camera::changeSampleSize(unsigned int size)
{
	film.changeSampleSize(size);
	filmSampleSize=size;
}
void Camera::changeSampler(WSampler::WSamplerType type)
{
	film.changeSampler(type);
}
float Camera::currProgress()
{
	unsigned int currX,currY;
	film.getCurrPos(currX,currY);
	Vector2 res=film.getResolution();
	float nPoints=currY*res.x+currX;
	float totalPoints=res.x*res.y;
	return nPoints/totalPoints;
}
int*Camera::getFilmBitPointer()
{
	return film.getBitPointer();
}

Vector3 Camera::getColor( unsigned int x,unsigned int y )
{
	return film.getColor(x,y);
}

void Camera::nextExposurePass()
{
	film.nextExposurePass();
}