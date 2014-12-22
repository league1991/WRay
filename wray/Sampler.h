#pragma once
#include "Sample.h"
class WSample;
class WSampler
{
public:
	enum WSamplerType{SAMPLER_RANDOM=0,SAMPLER_STRATIFIED=1};
	WSamplerType type;
	WSampler(unsigned int iseed=65536,WSamplerType itype=WSampler::SAMPLER_RANDOM);

	//纯虚函数，无论对于1D 2D还是3D 样本，
	//都是通过调用这个函数给样本分配范围在0-1之间的值
	//调用此函数时，首先会把sample的当前样本下标归0
	virtual void computeSamples(WSample&s)=0;
	void setSeed(unsigned int iseed){seed=iseed;}
	virtual ~WSampler(void);
protected:
	unsigned int seed;
};
//此Sampler用随机数对Sample赋值
//是最简单的采样器
class WRandomSampler:public WSampler
{
public:
	WRandomSampler(unsigned int iseed=65535):WSampler(iseed){}
	void computeSamples(WSample&s);
};
class WStratifiedSampler:public WSampler
{
public:
	WStratifiedSampler(unsigned int iseed=65535):WSampler(iseed){}
	void computeSamples(WSample&s);
private:
	void swapPoint(unsigned int ithPoint,unsigned int jthPoint,WSample&s);
};