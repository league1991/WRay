#pragma once
#include "Sampler.h"
#include "Sample.h"
#include "Vector3.h"
class WFilter
{
public:
	enum WFilterType{FILTER_BOX=0};
	WFilterType type;
	WFilter(unsigned int size=1,
		WSampler::WSamplerType sampType=WSampler::SAMPLER_RANDOM,
		unsigned int iseed=65535,
		WFilterType filterType=WFilter::FILTER_BOX);
	virtual ~WFilter(void);
	//改变样本点的个数
	void changeSampleSize(unsigned int isize);
	void changeSampler(WSampler::WSamplerType type=WSampler::SAMPLER_RANDOM);
	//此函数检查所有样本颜色是否获取完毕，
	//返回true表示获取完毕
	bool isFull();
	//以下两个函数必须配对使用
	//即每调用一次setSampleClr，必须调用一次getFinalClr

	//此函数通过采样颜色值计算一个像素的最终颜色
	virtual void getFinalClr(WVector3&final)=0;
	//此函数设置一个采样点的颜色
	void setSampleClr(WVector3 sampClr);
	//用sampler重新计算样本点,同时把样本的nthPoint成员归0
	//此函数直接调用Sampler的computeSamples接口，
	//computeSamples会先更新Sampler的种子值，
	//再计算随机采样点
	void refresh();
	//获得当前采样点的位置
	void getCurrSample(float &sampX,float&sampY);
protected:
	WSample2D samples;
	WSampler*sampler;
	WVector3*sampleColors;
	unsigned int seed;
};
//最简单的过滤器，把样本点简单取平均
class WBoxFilter:public WFilter
{
public:
	WBoxFilter(unsigned int size=1,
		WSampler::WSamplerType sampType=WSampler::SAMPLER_RANDOM,
		unsigned int iseed=65535):
	WFilter(size,sampType,iseed,WFilter::FILTER_BOX){}
	void getFinalClr(WVector3&final);
};