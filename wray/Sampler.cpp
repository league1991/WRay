#include "stdafx.h"
#include "Sampler.h"
#include <stdlib.h>

Sampler::Sampler(unsigned int iseed,SamplerType itype)
{
	seed=iseed;
	type=itype;
}

Sampler::~Sampler(void)
{
}
void RandomSampler::computeSamples(Sample&s)
{
	s.nthPoint=0;
	unsigned int nValues;
	if(s.type==Sample::SAMPLE_1D)
		nValues=s.totalPoints;
	else if(s.type==Sample::SAMPLE_2D)
		nValues=s.totalPoints*2;
	else
		nValues=s.totalPoints*3;
//	seed=time(0);
//	srand(seed);
	float inv32767 = 1.0f / 32767.0f;
	unsigned int i = 0;
	for(; i < (nValues & ~0x7);i+=8)
	{
		s.pattern[i]     = RandomNumber::randomFloat();
		s.pattern[i + 1] = RandomNumber::randomFloat();
		s.pattern[i + 2] = RandomNumber::randomFloat();
		s.pattern[i + 3] = RandomNumber::randomFloat();
		s.pattern[i + 4] = RandomNumber::randomFloat();
		s.pattern[i + 5] = RandomNumber::randomFloat();
		s.pattern[i + 6] = RandomNumber::randomFloat();
		s.pattern[i + 7] = RandomNumber::randomFloat();
//		cout<<s.pattern[i]<<endl;
	}
	for (;i < nValues; i++)
	{
		s.pattern[i]     = RandomNumber::randomFloat();
	}
}
void StratifiedSampler::swapPoint(unsigned int ithPoint,unsigned int jthPoint,Sample&s)
{
	if(s.type==Sample::SAMPLE_1D)
	{
		float temp=s.pattern[ithPoint];
		s.pattern[ithPoint]=s.pattern[jthPoint];
		s.pattern[jthPoint]=temp;
	}
	else if(s.type==Sample::SAMPLE_2D)
	{
		unsigned int ix,iy,jx,jy;
		ix=2*ithPoint;iy=ix+1;
		jx=2*jthPoint;jy=jx+1;

		float temp1,temp2;
		temp1=s.pattern[ix];temp2=s.pattern[iy];
		s.pattern[ix]=s.pattern[jx];s.pattern[iy]=s.pattern[jy];
		s.pattern[jx]=temp1;s.pattern[jy]=temp2;
	}
	else
	{
		unsigned int ix,iy,iz,jx,jy,jz;
		ix=3*ithPoint;iy=ix+1;iz=iy+1;
		jx=3*jthPoint;jy=jx+1;jz=jy+1;

		float temp1,temp2,temp3;
		temp1=s.pattern[ix];temp2=s.pattern[iy];temp3=s.pattern[iz];

		s.pattern[ix]=s.pattern[jx];
		s.pattern[iy]=s.pattern[jy];
		s.pattern[iz]=s.pattern[jz];
		s.pattern[jx]=temp1;s.pattern[jy]=temp2;s.pattern[jz]=temp3;
	}
}
void StratifiedSampler::computeSamples(Sample&s)
{
	s.nthPoint=0;
	if(s.type==Sample::SAMPLE_1D)
	{
		for(unsigned int i=0;i<s.totalPoints;i++)
			s.pattern[i]=(float(i)+RandomNumber::randomFloat())/s.size;
		for(unsigned int i=0;i<s.totalPoints;i++)
		{
			unsigned int nthPoint=RandomNumber::randomInt(s.totalPoints);
			swapPoint(i,nthPoint,s);
		}
	}
	else if(s.type==Sample::SAMPLE_2D)
	{
		for(unsigned int i=0;i<s.size;i++)
		{
			//i为x方向,j为y方向
			for(unsigned int j=0;j<s.size;j++)
			{
				//x坐标
				s.pattern[2*(s.size*j+i)]=
					(float(i)+RandomNumber::randomFloat())/s.size;
				//y坐标
				s.pattern[2*(s.size*j+i)+1]=
					(float(j)+RandomNumber::randomFloat())/s.size;
			}
		}
		for(unsigned int i=0;i<s.totalPoints;i++)
		{
			unsigned int nthPoint=RandomNumber::randomInt(s.totalPoints);
			swapPoint(i,nthPoint,s);
		}
	}
	else 
	{
		for(unsigned int i=0;i<s.size;i++)
		{
			//i为x方向,j为y方向,k为z方向
			for(unsigned int j=0;j<s.size;j++)
			{
				for(unsigned int k=0;k<s.size;k++)
				{
					//x坐标
					s.pattern[3*(s.size*s.size*k+s.size*j+i)]=
						(float(i)+RandomNumber::randomFloat())/s.size;
					//y坐标
					s.pattern[3*(s.size*s.size*k+s.size*j+i)+1]=	
						(float(j)+RandomNumber::randomFloat())/s.size;
					//z坐标
					s.pattern[3*(s.size*s.size*k+s.size*j+i)+2]=	
						(float(k)+RandomNumber::randomFloat())/s.size;
				}
			}
		}
		for(unsigned int i=0;i<s.totalPoints;i++)
		{
			unsigned int nthPoint=RandomNumber::randomInt(s.totalPoints);
			swapPoint(i,nthPoint,s);
		}
	}
}

void SequenceStratifiedSampler::setPixel(int x, int y)
{
	m_pixelX = x;
	m_pixelY = y;
}

void SequenceStratifiedSampler::setSampleIdx(int i)
{
	m_sampleIdx = max(0, min(i, m_dimension*m_dimension - 1));
}

void SequenceStratifiedSampler::computeSamples(Sample & s)
{
	s.nthPoint = 0;
	if (s.type == Sample::SAMPLE_1D)
	{
		for (unsigned int i = 0; i<s.totalPoints; i++)
			s.pattern[i] = (float(i) + float(rand()) / 32767.0f) / s.size;
		for (unsigned int i = 0; i<s.totalPoints; i++)
		{
			unsigned int nthPoint = RandomNumber::randomInt(s.totalPoints);
			swapPoint(i, nthPoint, s);
		}
	}
	else if (s.type == Sample::SAMPLE_2D)
	{
		for (unsigned int i = 0; i<s.size; i++)
		{
			//i为x方向,j为y方向
			for (unsigned int j = 0; j<s.size; j++)
			{
				//x坐标
				s.pattern[2 * (s.size*j + i)] = RandomNumber::randomFloat();
				//y坐标
				s.pattern[2 * (s.size*j + i) + 1] = RandomNumber::randomFloat();
			}
		}
		for (unsigned int i = 0; i<s.totalPoints; i++)
		{
			unsigned int nthPoint = RandomNumber::randomInt(s.totalPoints);
			swapPoint(i, nthPoint, s);
		}
	}
	else if (s.type == Sample::SAMPLE_3D)
	{
		for (unsigned int i = 0; i < s.size; i++)
		{
			//i为x方向,j为y方向,k为z方向
			for (unsigned int j = 0; j < s.size; j++)
			{
				for (unsigned int k = 0; k < s.size; k++)
				{
					s.pattern[3 * (s.size*s.size*k + s.size*j + i)] = RandomNumber::randomFloat();
					s.pattern[3 * (s.size*s.size*k + s.size*j + i) + 1] = RandomNumber::randomFloat();
					s.pattern[3 * (s.size*s.size*k + s.size*j + i) + 2] = RandomNumber::randomFloat();
				}
			}
		}
		for (unsigned int i = 0; i < s.totalPoints; i++)
		{
			unsigned int nthPoint = RandomNumber::randomInt(s.totalPoints);
			swapPoint(i, nthPoint, s);
		}
	}
	else if (s.type == Sample::SAMPLE_SEQUENCE_2D)
	{
		for (int i = 0; i < s.totalPoints; i++)
		{
			float inGridOffsetX = RandomNumber::randomFloat();
			float inGridOffsetY = RandomNumber::randomFloat();
			int gridOffsetX, gridOffsetY;
			computeOffset(i, gridOffsetX, gridOffsetY);

			s.pattern[2 * i] = (gridOffsetX + inGridOffsetX) / float(m_dimension);
			s.pattern[2 * i+1] = (gridOffsetY + inGridOffsetY) / float(m_dimension);
		}
	}
}

void SequenceStratifiedSampler::swapPoint(unsigned int ithPoint, unsigned int jthPoint, Sample & s)
{
	if (s.type == Sample::SAMPLE_1D)
	{
		float temp = s.pattern[ithPoint];
		s.pattern[ithPoint] = s.pattern[jthPoint];
		s.pattern[jthPoint] = temp;
	}
	else if (s.type == Sample::SAMPLE_2D)
	{
		unsigned int ix, iy, jx, jy;
		ix = 2 * ithPoint; iy = ix + 1;
		jx = 2 * jthPoint; jy = jx + 1;

		float temp1, temp2;
		temp1 = s.pattern[ix]; temp2 = s.pattern[iy];
		s.pattern[ix] = s.pattern[jx]; s.pattern[iy] = s.pattern[jy];
		s.pattern[jx] = temp1; s.pattern[jy] = temp2;
	}
	else
	{
		unsigned int ix, iy, iz, jx, jy, jz;
		ix = 3 * ithPoint; iy = ix + 1; iz = iy + 1;
		jx = 3 * jthPoint; jy = jx + 1; jz = jy + 1;

		float temp1, temp2, temp3;
		temp1 = s.pattern[ix]; temp2 = s.pattern[iy]; temp3 = s.pattern[iz];

		s.pattern[ix] = s.pattern[jx];
		s.pattern[iy] = s.pattern[jy];
		s.pattern[iz] = s.pattern[jz];
		s.pattern[jx] = temp1; s.pattern[jy] = temp2; s.pattern[jz] = temp3;
	}
}
