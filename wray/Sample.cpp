#include "stdafx.h"
#include "Sample.h"

WSample::WSample(WSampleType dimension,unsigned int isize)
{
	type=dimension;
	if(type==SAMPLE_1D)
		totalPoints=isize;
	else if(type==SAMPLE_2D)
		totalPoints=isize*isize;
	else
		totalPoints=isize*isize*isize;
	nthPoint=0;
	size=isize;
	pattern=NULL;
}
WSample::~WSample(void)
{
	delete []pattern;
}
void WSample::clear()
{
	delete[]pattern;
	pattern=NULL;
	nthPoint=totalPoints=0;
}
void WSample::setSize(unsigned int size)
{
	if(type==SAMPLE_1D)
		totalPoints=size;
	else if(type==SAMPLE_2D)
		totalPoints=size*size;
	else
		totalPoints=size*size*size;
}
WSample1D::WSample1D(unsigned int size):WSample(SAMPLE_1D,size){}
void WSample1D::allocateSpace()
{
	pattern=new float[totalPoints];
	nthPoint=0;
}
bool WSample1D::get1D(float &x)
{
	if(nthPoint<totalPoints)
	{
		x=pattern[nthPoint];
		nthPoint++;
		return true;
	}
	return false;
}
void WSample1D::display()
{
	for(unsigned int i=0;i<totalPoints;i++)
		cout<<pattern[i]<<endl;
}
WSample2D::WSample2D(unsigned int size):WSample(SAMPLE_2D,size){}
void WSample2D::allocateSpace()
{
	pattern=new float[totalPoints*2];
	nthPoint=0;
}
bool WSample2D::get2D(float&x,float&y)
{
	if(nthPoint<totalPoints)
	{
		x=pattern[2*nthPoint];
		y=pattern[2*nthPoint+1];
		nthPoint++;return true;
	}
	return false;
}
void WSample2D::display()
{
	for(unsigned int i=0;i<totalPoints;i++)
		cout<<pattern[2*i]<<" "<<pattern[2*i+1]<<endl;
}
WSample3D::WSample3D(unsigned int size):WSample(SAMPLE_3D,size){}
void WSample3D::allocateSpace()
{
	pattern=new float[totalPoints*3];
	nthPoint=0;
}
bool WSample3D::get3D(float&x,float&y,float&z)
{
	if(nthPoint<totalPoints)
	{
		x=pattern[3*nthPoint];
		y=pattern[3*nthPoint+1];
		z=pattern[3*nthPoint+2];
		nthPoint++;return true;
	}
	return false;
}
void WSample3D::display()
{
	for(unsigned int i=0;i<totalPoints;i++)
		cout<<pattern[3*i]<<' '<<pattern[3*i+1]<<' '<<pattern[3*i+2]<<endl;
}