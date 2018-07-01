#include "stdafx.h"
#include "MC.h"
#include <math.h>

//PCGRandomObj RandomNumber::s_PCGRandObj(12345,678);
//std::mt19937 RandomNumber::s_randObj(12345);
//std::uniform_real_distribution<float> RandomNumber::s_uniformFloatObj;
//std::uniform_int_distribution<> RandomNumber::s_uniformIntObj;
RandomNumber* RandomNumber::s_globalObj = new RandomNumber();
std::unordered_map<std::thread::id, RandomNumber*> RandomNumber::s_threadRNG;
std::mutex RandomNumber::s_lock;

RandomNumber::RandomNumber(void)
{
}

RandomNumber::~RandomNumber(void)
{
}

float RandomNumber::randomFloat()
{
    //return rand() / 32767.0;
    //return s_uniformFloatObj(s_randObj);
    uint32_t s = m_PCGRandObj.pcgRandom();
    return min(0.99999994f, float(s * 2.3283064365386963e-10f));
    //return s / float(INT32_MAX);
}

int RandomNumber::randomInt(int count)
{
    //return rand() % count;
    //return s_uniformIntObj(s_randObj) % count;
    return m_PCGRandObj.pcgRandom() % count;
}

inline void RandomNumber::randomSeed(unsigned int seed)
{
    //srand(seed);
    //s_randObj.seed(seed);
    m_PCGRandObj.m_inc = seed;
    m_PCGRandObj.m_state = 12345;
}

RandomNumber * RandomNumber::getGlobalObj()
{
    auto id = std::this_thread::get_id();
    auto rngIt = s_threadRNG.find(id);
    if (rngIt == s_threadRNG.end())
    {
        auto rng = new RandomNumber();
        std::lock_guard<std::mutex> lock(s_lock);
        s_threadRNG[id] = rng;
        return rng;
    }
    return rngIt->second;
    //return s_globalObj;
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
	sample.z=sqrt(max(1-sample.x*sample.x-sample.y*sample.y,0.01f));
	PDF=sample.z*M_INV_PI;
}
void RandomNumber::uniformSampleSphere(float u1, float u2, Vector3 & sample, float & PDF)
{
    float cosTheta = 1 - 2.0f * u2;
    float sinTheta = sqrt(1 - cosTheta * cosTheta);
    float phi = 2 * M_PI * u1;
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);
    sample.x = cosPhi * sinTheta;
    sample.y = sinPhi * sinTheta;
    sample.z = cosTheta;
    PDF = 1.0f / (4.0f * M_PI);
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
