#include "stdafx.h"
#include "Sampler.h"
#include <stdlib.h>

WSampler::WSampler(unsigned int iseed,WSamplerType itype)
{
	seed=iseed;
	type=itype;
}

WSampler::~WSampler(void)
{
}
void WRandomSampler::computeSamples(WSample&s)
{
	s.nthPoint=0;
	unsigned int nValues;
	if(s.type==WSample::SAMPLE_1D)
		nValues=s.totalPoints;
	else if(s.type==WSample::SAMPLE_2D)
		nValues=s.totalPoints*2;
	else
		nValues=s.totalPoints*3;
//	seed=time(0);
//	srand(seed);
	float inv32767 = 1.0f / 32767.0f;
	unsigned int i = 0;
	for(; i < (nValues & ~0x7);i+=8)
	{
		s.pattern[i]     = float(rand()) * inv32767;
		s.pattern[i + 1] = float(rand()) * inv32767;
		s.pattern[i + 2] = float(rand()) * inv32767;
		s.pattern[i + 3] = float(rand()) * inv32767;
		s.pattern[i + 4] = float(rand()) * inv32767;
		s.pattern[i + 5] = float(rand()) * inv32767;
		s.pattern[i + 6] = float(rand()) * inv32767;
		s.pattern[i + 7] = float(rand()) * inv32767;
//		cout<<s.pattern[i]<<endl;
	}
	for (;i < nValues; i++)
	{
		s.pattern[i]     = float(rand()) * inv32767;
	}
}
void WStratifiedSampler::swapPoint(unsigned int ithPoint,unsigned int jthPoint,WSample&s)
{
	if(s.type==WSample::SAMPLE_1D)
	{
		float temp=s.pattern[ithPoint];
		s.pattern[ithPoint]=s.pattern[jthPoint];
		s.pattern[jthPoint]=temp;
	}
	else if(s.type==WSample::SAMPLE_2D)
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
void WStratifiedSampler::computeSamples(WSample&s)
{
	s.nthPoint=0;
	if(s.type==WSample::SAMPLE_1D)
	{
		for(unsigned int i=0;i<s.totalPoints;i++)
			s.pattern[i]=(float(i)+float(rand())/32767.0f)/s.size;
		for(unsigned int i=0;i<s.totalPoints;i++)
		{
			unsigned int nthPoint=rand()%s.totalPoints;
			swapPoint(i,nthPoint,s);
		}
	}
	else if(s.type==WSample::SAMPLE_2D)
	{
		for(unsigned int i=0;i<s.size;i++)
		{
			//i为x方向,j为y方向
			for(unsigned int j=0;j<s.size;j++)
			{
				//x坐标
				s.pattern[2*(s.size*j+i)]=
					(float(i)+float(rand())/32767.0f)/s.size;
				//y坐标
				s.pattern[2*(s.size*j+i)+1]=
					(float(j)+float(rand())/32767.0f)/s.size;
			}
		}
		for(unsigned int i=0;i<s.totalPoints;i++)
		{
			unsigned int nthPoint=rand()%s.totalPoints;
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
						(float(i)+float(rand())/32767.0f)/s.size;
					//y坐标
					s.pattern[3*(s.size*s.size*k+s.size*j+i)+1]=	
						(float(j)+float(rand())/32767.0f)/s.size;
					//z坐标
					s.pattern[3*(s.size*s.size*k+s.size*j+i)+2]=	
						(float(k)+float(rand())/32767.0f)/s.size;
				}
			}
		}
		for(unsigned int i=0;i<s.totalPoints;i++)
		{
			unsigned int nthPoint=rand()%s.totalPoints;
			swapPoint(i,nthPoint,s);
		}
	}
}