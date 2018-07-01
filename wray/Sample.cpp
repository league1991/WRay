#include "stdafx.h"
#include "Sample.h"

Sample::Sample(SampleType dimension,unsigned int isize)
{
	type=dimension;
	if (type == SAMPLE_1D || type == SAMPLE_SEQUENCE_2D)
		totalPoints = isize;
	else if (type == SAMPLE_2D)
		totalPoints = isize*isize;
	else
		totalPoints = isize*isize*isize;
	nthPoint=0;
	size=isize;
	pattern.clear();
}
Sample::~Sample(void)
{
}
void Sample::clear()
{
	nthPoint=totalPoints=0;
}
void Sample::setSize(unsigned int isize)
{
    size = isize;
	if (type == SAMPLE_1D || type == SAMPLE_SEQUENCE_2D)
		totalPoints = size;
	else if (type == SAMPLE_2D)
		totalPoints = size*size;
	else
		totalPoints = size*size*size;
}
Sample1D::Sample1D(unsigned int size):Sample(SAMPLE_1D,size){}
void Sample1D::allocateSpace()
{
	pattern.resize(totalPoints);
	nthPoint=0;
}
bool Sample1D::get1D(float &x)
{
	if(nthPoint<totalPoints)
	{
		x=pattern[nthPoint];
		nthPoint++;
		return true;
	}
	else
	{
		x = RandomNumber::getGlobalObj()->randomFloat();
		return false;
	}
}
void Sample1D::display()
{
	for(unsigned int i=0;i<totalPoints;i++)
		cout<<pattern[i]<<endl;
}
Sample2D::Sample2D(unsigned int size):Sample(SAMPLE_2D,size){}
void Sample2D::allocateSpace()
{
	pattern.resize(totalPoints*2);
	nthPoint=0;
}
bool Sample2D::get2D(float&x,float&y)
{
	if(nthPoint<totalPoints)
	{
		x=pattern[2*nthPoint];
		y=pattern[2*nthPoint+1];
		nthPoint++;
		return true;
	}
	else
	{
		x = RandomNumber::getGlobalObj()->randomFloat();
		y = RandomNumber::getGlobalObj()->randomFloat();
		return false;
	}
}
void Sample2D::display()
{
	for(unsigned int i=0;i<totalPoints;i++)
		cout<<pattern[2*i]<<" "<<pattern[2*i+1]<<endl;
}
Sample3D::Sample3D(unsigned int size):Sample(SAMPLE_3D,size){}
void Sample3D::allocateSpace()
{
	pattern.resize(totalPoints * 3);
	nthPoint=0;
}
bool Sample3D::get3D(float&x,float&y,float&z)
{
	if(nthPoint<totalPoints)
	{
		x=pattern[3*nthPoint];
		y=pattern[3*nthPoint+1];
		z=pattern[3*nthPoint+2];
		nthPoint++;return true;
	}
	else
	{
		x = RandomNumber::getGlobalObj()->randomFloat();
		y = RandomNumber::getGlobalObj()->randomFloat();
		z = RandomNumber::getGlobalObj()->randomFloat();
		return false;
	}
}
void Sample3D::display()
{
	for(unsigned int i=0;i<totalPoints;i++)
		cout<<pattern[3*i]<<' '<<pattern[3*i+1]<<' '<<pattern[3*i+2]<<endl;
}

void SequenceSample2D::allocateSpace()
{
	pattern.resize(totalPoints * 2);
	nthPoint = 0;
}

bool SequenceSample2D::get2D(float & x, float & y)
{
	if (nthPoint<totalPoints)
	{
		x = pattern[2 * nthPoint];
		y = pattern[2 * nthPoint + 1];
		nthPoint++;
		return true;
	}
	else
	{
		x = RandomNumber::getGlobalObj()->randomFloat();
		y = RandomNumber::getGlobalObj()->randomFloat();
		return false;
	}
}
