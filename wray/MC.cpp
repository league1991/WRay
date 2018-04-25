#include "stdafx.h"
#include "MC.h"
#include <math.h>
RandomNumber::RandomNumber(void)
{
}

RandomNumber::~RandomNumber(void)
{
}
float RandomNumber::randomFloat()
{
	return float(rand())/32767.0f;
}
void RandomNumber::uniformSampleDisk(const float u1, const float u2, float &x, float &y)
{
	float r=sqrt(u1);
	float theta=2.0f*M_PI*u2;
	x=r*cos(theta);
	y=r*sin(theta);
}
void RandomNumber::cosineSampleHemisphere(const float u1, const float u2, Vector3 &sample,float&PDF)
{
	uniformSampleDisk(u1,u2,sample.x,sample.y);
	sample.z=sqrt(max(1-sample.x*sample.x-sample.y*sample.y,0.001));
	PDF=sample.z*M_INV_PI;
}
void RandomNumber::uniformSampleTriangle(float u1, float u2, float &u, float &v)
{
	float squrU1=sqrt(u1);
	u=1-squrU1;
	v=u2*squrU1;
}

float RandomNumber::powerHeuristic(int nf, float fPdf, int ng, float gPdf)
{
	float f = nf * fPdf;
	float g = ng * gPdf;
	return (f*f) / (f*f + g*g);
}
