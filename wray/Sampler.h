#pragma once
#include "Sample.h"
class Sample;
class Sampler
{
public:
	enum SamplerType
	{
		SAMPLER_RANDOM=0,
		SAMPLER_STRATIFIED=1,
		SAMPLER_SEQUENCE_STRATIFIED=2
	};
	SamplerType type;
	Sampler(unsigned int iseed=65536,SamplerType itype=Sampler::SAMPLER_RANDOM);

	//纯虚函数，无论对于1D 2D还是3D 样本，
	//都是通过调用这个函数给样本分配范围在0-1之间的值
	//调用此函数时，首先会把sample的当前样本下标归0
	virtual void computeSamples(Sample&s)=0;
	void setSeed(unsigned int iseed){seed=iseed;}
	virtual ~Sampler(void);
protected:
	unsigned int seed;
};
//此Sampler用随机数对Sample赋值
//是最简单的采样器
class RandomSampler:public Sampler
{
public:
	RandomSampler(unsigned int iseed=65535):Sampler(iseed, SAMPLER_RANDOM){}
	void computeSamples(Sample&s);
};
class StratifiedSampler:public Sampler
{
public:
	StratifiedSampler(unsigned int iseed=65535):Sampler(iseed, SAMPLER_STRATIFIED){}
	void computeSamples(Sample&s);
private:
	void swapPoint(unsigned int ithPoint,unsigned int jthPoint,Sample&s);
};

class SequenceStratifiedSampler : public Sampler
{
public:
	SequenceStratifiedSampler(unsigned int seed = 65535, int dimension = 4):
		Sampler(seed, SAMPLER_SEQUENCE_STRATIFIED), m_dimension(dimension), m_pixelX(0), m_pixelY(0) {}

	void setPixel(int x, int y);
	void setSampleIdx(int i);
	void setDimension(int dimension) { m_dimension = dimension; }

	int getNumPixelSamples() const { return m_dimension*m_dimension; }

	void computeSamples(Sample&s);
private:
	void swapPoint(unsigned int ithPoint, unsigned int jthPoint, Sample&s);

	void computeOffset(int ithSlice, int& xOffset, int& yOffset)
	{
		int offset = m_pixelX * 12345 + m_pixelY * 54321 + ithSlice * 13579;
		int gridCount = m_dimension * m_dimension;
		int gridOffset = (offset + m_sampleIdx) % gridCount;
		xOffset = gridOffset % m_dimension;
		yOffset = gridOffset / m_dimension;
	}

	int m_pixelX, m_pixelY;
	int m_sampleIdx;
	int m_dimension;
};
