#include "stdafx.h"
#include "Filter.h"

WFilter::WFilter(unsigned int size,
			   WSampler::WSamplerType sampType,
			   unsigned int iseed,
			   WFilterType filterType):samples(size)
{
	type=filterType;
	seed=iseed;
	sampleColors=new WVector3[size*size];//样本颜色
	samples.allocateSpace();
	if(sampType==WSampler::SAMPLER_RANDOM)
		sampler=new WRandomSampler(iseed);
	sampler->computeSamples(samples);//计算采样点
}

WFilter::~WFilter(void)
{
	delete[]sampleColors;
	delete sampler;
}
void WFilter::changeSampleSize(unsigned int isize)
{
	samples.clear();
	samples.setSize(isize);
	samples.allocateSpace();
	delete []sampleColors;
	sampleColors=new WVector3[isize*isize];
}
void WFilter::changeSampler(WSampler::WSamplerType type)
{
	delete sampler;
	if(type==WSampler::SAMPLER_RANDOM)
		sampler=new WRandomSampler(seed);
}
void WFilter::setSampleClr(WVector3 sampClr)
{
	sampleColors[samples.nthPoint]=sampClr;
	samples.nthPoint++;
	return ;
}
void WFilter::refresh()
{
	sampler->computeSamples(samples);
}
void WBoxFilter::getFinalClr(WVector3&final)
{
	WVector3 totalClr=WVector3(0,0,0);
	for(unsigned int i=0;i<samples.totalPoints;i++)
	{
//		cout<<i<<endl;
		totalClr+=sampleColors[i];
	}
	final=totalClr/float(samples.totalPoints);
}
bool WFilter::isFull()
{
	if(samples.nthPoint==samples.totalPoints)
		return true;
		return false;
}
void WFilter::getCurrSample(float &sampX, float&sampY)
{
	sampX=samples.pattern[2*samples.nthPoint];
	sampY=samples.pattern[2*samples.nthPoint+1];
}
