// WAccelerator.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Accelerator.h"


WAccelerator::WAccelerator(void)
{
	resetStatistics();
}

WAccelerator::~WAccelerator(void)
{
}

void* WAccelerator::allocAligned16( const unsigned int bytes )
{
	const int ptrSize = sizeof(byte*);
	byte* ptr = (byte *) malloc( bytes + 16 + ptrSize);
	if (!ptr)
		return NULL;

	byte* alignedPtr = (byte *) ( ( (size_t) ptr ) + 15 & ~15ULL );
	if ( alignedPtr - ptr < ptrSize) {
		alignedPtr += 16;
	}
	*((size_t *)(alignedPtr - ptrSize)) = (size_t) ptr;
	return (void *) alignedPtr;
}
void WAccelerator::freeAligned16(void* ptr)
{
	const int ptrSize = sizeof(byte*);
	free((void *) *((size_t *) ( ((byte*)ptr) - ptrSize)));
}

void WAccelerator::resetStatistics()
{
	numIntersectTest = numIntersect = 
		numTraverseInterior_IT = numTraverseLeaf_IT = numTriangleIsect_IT = 
		numTraverseInterior_I = numTraverseLeaf_I = numTriangleIsect_I = 0;
}

void WAccelerator::displayIsectStatistics()
{
	if(numIntersectTest == 0)
		printf("\nno intersect test statistics\n");
	else
	{
		printf("\n----Intersect Test Statistics----\n");
		printf("total intersect test           : %d\n", numIntersectTest);
		printf("average interior node traversed: %f\n", (float)numTraverseInterior_IT / numIntersectTest);
		printf("average leaf node traversed    : %f\n", (float)numTraverseLeaf_IT / numIntersectTest);
		printf("average triangle intersect     : %f\n", (float)numTriangleIsect_IT / numIntersectTest);
	}	
	if(numIntersect == 0)
		printf("no intersect statistics\n");
	else
	{
		printf("------Intersect Statistics-------\n");
		printf("total intersect                : %d\n", numIntersect);
		printf("average interior node traversed: %f\n", (float)numTraverseInterior_I / numIntersect);
		printf("average leaf node traversed    : %f\n", (float)numTraverseLeaf_I / numIntersect);
		printf("average triangle intersect     : %f\n", (float)numTriangleIsect_I / numIntersect);
	}
}