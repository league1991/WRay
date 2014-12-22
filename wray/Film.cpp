#include "StdAfx.h"
#include "Film.h"


WFilm::WFilm(unsigned int resX,unsigned int resY,
		   unsigned int sampleSize,
		   WFilter::WFilterType filterType,
		   WSampler::WSamplerType samplerType,
		   unsigned int seed)
{
	resolutionX=resX;
	resolutionY=resY;
	//用于存放整个画面的所有颜色数据,其长度是像素点个数x3.
	colors=new float[resolutionX*resolutionY*3];  
	//作用暂时不明
	bitColors=new int[resolutionX*resolutionY];
	//作用暂时不明
	counts = new float[resolutionX*resolutionY];
	currColor=colors;
	currX=currY=0;
	if(filterType==WFilter::FILTER_BOX)
	{
		filter=new WBoxFilter(sampleSize,samplerType,seed);
	}
}
WFilm::~WFilm(void)
{
	delete[]colors;
	delete[]bitColors;
	delete[]counts;
	delete filter;
}
void WFilm::buildColorArray()
{
	delete []colors;
	delete []bitColors;
	delete []counts;
	colors = new float[resolutionX*resolutionY*3];
	bitColors=new int[resolutionX*resolutionY];
	counts = new float[resolutionX*resolutionY];
	for (unsigned int i = 0; i < resolutionX*resolutionY; i++)
		counts[i] = 0.0f;
	currColor=colors;
}
void WFilm::setResolution(unsigned int resX,unsigned int resY)
{
	resolutionX=resX;
	resolutionY=resY;
	buildColorArray();
}
void WFilm::cleanColors(float R,float G,float B)
{
	if(!(resolutionX>0&&resolutionY>0))
		return;
	unsigned int totalColors=resolutionX*resolutionY;
	for(unsigned int nthColor=0;nthColor<totalColors;nthColor++)
	{
		colors[3*nthColor]=R;
		colors[3*nthColor+1]=G;
		colors[3*nthColor+2]=B;
		counts[nthColor] = 0.0f;
		bitColors[nthColor]=float2int(R,G,B);
	}
}
void WFilm::draw(float x,float y,float z)
{
	glRasterPos3f(x,y,z);
	glDrawPixels(resolutionX,resolutionY,
		GL_RGB,GL_FLOAT,colors);
}
void WFilm::setColor(unsigned int x,unsigned int y,float R,float G,float B)
{
//	printf("RGB is %f %f %f",R,G,B);
	colors[3*(x+resolutionX*y)]=R;
	colors[3*(x+resolutionX*y)+1]=G;
	colors[3*(x+resolutionX*y)+2]=B;
	counts[x+resolutionX*y] = 1.0f;
	bitColors[x+resolutionX*y] = float2int(R,G,B);
}

void WFilm::accumulateColor(unsigned int x,unsigned int y,float R,float G,float B)
{
	float* color = colors + 3*(x+resolutionX*y);
	float & count = counts[x+resolutionX*y];
	color[0] = (color[0] * count + R) / (count + 1);
	color[1] = (color[1] * count + G) / (count + 1);
	color[2] = (color[2] * count + B) / (count + 1);
	count++;
}
WVector3 WFilm::getColor(unsigned int x,unsigned int y)
{
	return WVector3(colors[3*(x+resolutionX*y)],
		colors[3*(x+resolutionX*y)+1],
		colors[3*(x+resolutionX*y)+2]);
}
void WFilm::setSampleColor(float R, float G, float B)
{
	float* pixelColor = &colors[3*(currX+resolutionX*currY)];
	float* count = &counts[currX+resolutionX*currY];
	float  newCount = *count + 1;
	*pixelColor = (*pixelColor * (*count) + R) / newCount;
	pixelColor++;
	*pixelColor = (*pixelColor * (*count) + G) / newCount;
	pixelColor++;
	*pixelColor = (*pixelColor * (*count) + B) / newCount;
	*count = newCount;
	currX++;
	if(currX==resolutionX)
	{
		currX=0;currY++;
	}
}
bool WFilm::isFull()
{
	return currY==resolutionY;
}
void WFilm::nextExposurePass()
{
	currX = currY = 0;
}
void WFilm::changeSampleSize(unsigned int size)
{
	filter->changeSampleSize(size);
}
void WFilm::changeSampler(WSampler::WSamplerType type)
{
	filter->changeSampler(type);
}
void WFilm::getSamplePosition(float&posX,float&posY)
{
//	filter->getCurrSample(posX,posY);
	posX=posY=0.0f;
	posX+=currX;
	posY+=currY;
}
int WFilm::float2int(float&r,float&g,float&b)
{
	return 
		(((short)min(255.0f,r*255.0f))<<16)|
		(((short)min(255.0f,g*255.0f))<<8)|
		((short)min(255.0f,b*255.0f));
}
