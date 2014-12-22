#include "StdAfx.h"
#include "RandomCamera.h"

WRandomCamera::WRandomCamera(void)
{
	nSamples = NULL;
}

WRandomCamera::~WRandomCamera(void)
{
	delete[]nSamples;
}

void WRandomCamera::setFilmResolutionX( unsigned int resX )
{
//	cout<<"set resolution in randomCamera."<<endl;
	//除了改变胶片的分辨率，还要另外新建一个数组
	WCamera::setFilmResolutionX(resX);
	unsigned int resY = (unsigned int)ceil(resX/ratio);
	
	delete []nSamples;
	nSamples = new unsigned int[resX*resY];
	for(unsigned int ithPixel=0; ithPixel<resX*resY; ithPixel++)
		nSamples[ithPixel]=0;
	
	this->resolutionX = resX;
	this->resolutionY = resY;
}

void WRandomCamera::setFilmResolutionXY( unsigned int resX,unsigned int resY )
{
	//除了改变胶片的分辨率，还要另外新建一个数组
	WCamera::setFilmResolutionXY(resX, resY);
	
	delete []nSamples;
	nSamples = new unsigned int[resX*resY];
	for(unsigned int ithPixel=0; ithPixel<resX*resY; ithPixel++)
		nSamples[ithPixel]=0;

	resolutionX = resX;
	resolutionY = resY;
}

void WRandomCamera::changeSampleSize( unsigned int size )
{
	film.changeSampleSize(1);
}

void WRandomCamera::setColor( float R,float G,float B, WVector3 idir )
{
	//nDir为屏幕中心到idir在屏幕上的中心投影点的向量
	WVector3 nDir = idir / (idir.dot(this->dir)) - dir;
	float xRatio = nDir.dot(this->x) / x.lengthSquared();
	float yRatio = nDir.dot(this->y) / y.lengthSquared();
	
	unsigned int xPos = (xRatio + 1.0f) / 2.0f * this->resolutionX;
	unsigned int yPos = (yRatio + 1.0f) / 2.0f * this->resolutionY;
	
	if(xPos>=resolutionX || yPos>=resolutionY)
		return;
	unsigned int* currSamplePtr = &(this->nSamples[xPos + resolutionX * yPos]);
	WVector3 currColor = film.getColor(xPos, yPos) * (*currSamplePtr);
	currColor = (currColor + WVector3(R, G, B)) / (float)(++(*currSamplePtr));
//	cout<<*currSamplePtr<<endl;
//	printf("the position in screen is %d %d \n",xPos,yPos);
	film.setColor(xPos, yPos, currColor.x, currColor.y, currColor.z);
	
}

void WRandomCamera::clearFilm( float R /*=0.0f*/,float G /*=0.0f*/,float B /*=0.0f*/ )
{
	WCamera::clearFilm(R, G, B);
	for(unsigned int ithPixel=0; ithPixel<resolutionX*resolutionY; ithPixel++)
		nSamples[ithPixel]=0;
	
}