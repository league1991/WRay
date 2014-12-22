#pragma once

class WRecursivePathIntegrator:public WSurfaceIntegrator
{
public:
	WRecursivePathIntegrator(WScene*scene,WAccelerator*tree,
		unsigned int ipathDepth=1,unsigned int inBranch=2,
		WSampler::WSamplerType samplerType=WSampler::SAMPLER_RANDOM,float imultiplier=1);
	virtual ~WRecursivePathIntegrator(void);

	//光线追踪函数
	WVector3 integrate(WRay&ray);
	void setPathMaxDepth(unsigned int idepth);
	void displayTime();
private:
	WClock timer;
	WSampler*sampler;
	WSample2D BSDFSamples;
	WSample3D lightSamples;
	//计算直接光照的对象
	WDirectLighting Dlighting;
	//树的深度
	unsigned int maxDepth;
	//分支数
	unsigned int nBranchs;
	float multiplier;
	void clearSamples();
	void allocateSamples();
	void computeSamples();
	//光线追踪递归函数
	WVector3 integrateCore(WRay ray,unsigned int depth,int beginNode = 0);
};
