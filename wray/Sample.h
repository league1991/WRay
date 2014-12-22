#pragma once
#include "Sampler.h"
class WSample
{
	friend class WSampler;
	friend class WRandomSampler;
	friend class WStratifiedSampler;

	friend class WFilter;
	friend class WBoxFilter;
public:
	enum WSampleType{SAMPLE_1D=0,SAMPLE_2D=1,SAMPLE_3D=2};
	WSampleType type;
	//构造的时候并不分配空间
	WSample(WSampleType dimension,unsigned int size);
	virtual ~WSample(void);
	virtual void allocateSpace()=0;//分配内存空间,同时nthPoint设为0
	void clear();//清理内存空间
//	void reset();//把当前下标复位
	void setSize(unsigned int size);
	void setType(WSampleType itype){type=itype;}
	unsigned int getCurrPointIndex(){return nthPoint;}
	unsigned int getTotalPointNum(){return totalPoints;}
	virtual void display()=0;
protected:
	unsigned int size;
	float*pattern;
	unsigned int nthPoint;//当前的采样点
	unsigned int totalPoints;//采样点的总数
};
class WSample1D:public WSample
{
public:
	WSample1D(unsigned int size);
	void allocateSpace();
	bool get1D(float&x);
	void display();
};
class WSample2D:public WSample
{
public:
	WSample2D(unsigned int size);
	void allocateSpace();
	bool get2D(float&x,float&y);
	void display();
};
class WSample3D:public WSample
{
public:
	WSample3D(unsigned int size);
	void allocateSpace();
	bool get3D(float&x,float&y,float&z);
	void display();
};