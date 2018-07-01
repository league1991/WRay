#pragma once
#include "surfaceintegrator.h"

class PathIntegrator :
	public SurfaceIntegrator
{
public:
	PathIntegrator(Scene*scene,WAccelerator*tree,
		unsigned int ipathDepth=1,
		Sampler::SamplerType samplerType=Sampler::SAMPLER_RANDOM,float imultiplier=1);
	virtual ~PathIntegrator(void);

	//¹âÏß×·×Ùº¯Êý
	Vector3 integrate(Ray&ray);

	// prepare samples
	void initSamples(int nSampleGroup);
	void setPathMaxDepth(unsigned int idepth);
	void displayTime();
private:
	WClock timer;
	Sampler*sampler;
	SequenceSample2D BSDFSamples;
	Sample3D lightSamples;

	vector<Sample2D> BSDFSampleGroup;
	DirectLighting Dlighting;
	unsigned int pathMaxDepth;
	float multiplier;
	void clearSamples();
	void allocateSamples();
	void computeSamples();
};
