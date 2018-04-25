#include "stdafx.h"
#include "Filter.h"

WFilter::WFilter(unsigned int size,
			   Sampler::SamplerType sampType,
			   unsigned int iseed,
			   WFilterType filterType):samples(size)
{
	type=filterType;
	seed=iseed;
	sampleColors=new Vector3[size*size];//样本颜色
	samples.allocateSpace();
	if(sampType==Sampler::SAMPLER_RANDOM)
		sampler=new RandomSampler(iseed);
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
	sampleColors=new Vector3[isize*isize];
}
void WFilter::changeSampler(Sampler::SamplerType type)
{
	delete sampler;
	if(type==Sampler::SAMPLER_RANDOM)
		sampler=new RandomSampler(seed);
}
void WFilter::setSampleClr(Vector3 sampClr)
{
	sampleColors[samples.nthPoint]=sampClr;
	samples.nthPoint++;
	return ;
}
void WFilter::refresh()
{
	sampler->computeSamples(samples);
}
void WBoxFilter::getFinalClr(Vector3&final)
{
	Vector3 totalClr=Vector3(0,0,0);
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
