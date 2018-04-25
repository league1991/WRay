#pragma once
#include "Sampler.h"
class Sample
{
	friend class Sampler;
	friend class RandomSampler;
	friend class StratifiedSampler;
	friend class SequenceStratifiedSampler;

	friend class WFilter;
	friend class WBoxFilter;
public:
	enum SampleType
	{
		SAMPLE_1D=0,
		SAMPLE_2D=1,
		SAMPLE_3D=2,
		SAMPLE_SEQUENCE_2D = 3,
	};
	SampleType type;
	//构造的时候并不分配空间
	Sample(SampleType dimension,unsigned int size);
	virtual ~Sample(void);
	virtual void allocateSpace()=0;//分配内存空间,同时nthPoint设为0
	void clear();//清理内存空间
//	void reset();//把当前下标复位
	void setSize(unsigned int size);
	void setType(SampleType itype){type=itype;}
	unsigned int getCurrPointIndex(){return nthPoint;}
	unsigned int getTotalPointNum(){return totalPoints;}
	virtual void display() {};
protected:
	unsigned int size;
	std::vector<float> pattern;
	unsigned int nthPoint;//当前的采样点
	unsigned int totalPoints;//采样点的总数
};
class Sample1D:public Sample
{
public:
	Sample1D(unsigned int size);
	void allocateSpace();
	bool get1D(float&x);
	void display();
};
class Sample2D:public Sample
{
public:
	Sample2D(unsigned int size);
	void allocateSpace();
	virtual bool get2D(float&x,float&y);
	void display();
};
class Sample3D:public Sample
{
public:
	Sample3D(unsigned int size);
	void allocateSpace();
	bool get3D(float&x,float&y,float&z);
	void display();
};
class SequenceSample2D :public Sample2D
{
public:
	SequenceSample2D(unsigned int size) :Sample2D(size) {
		type = SAMPLE_SEQUENCE_2D;
		totalPoints	= size;
	}
	void allocateSpace();
	bool get2D(float&x, float&y);
};
