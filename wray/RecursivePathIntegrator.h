#pragma once

class WRecursivePathIntegrator:public WSurfaceIntegrator
{
public:
	WRecursivePathIntegrator(Scene*scene,WAccelerator*tree,
		unsigned int ipathDepth=1,unsigned int inBranch=2,
		Sampler::SamplerType samplerType=Sampler::SAMPLER_RANDOM,float imultiplier=1);
	virtual ~WRecursivePathIntegrator(void);

	//光线追踪函数
	Vector3 integrate(Ray&ray);
	void setPathMaxDepth(unsigned int idepth);
	void displayTime();
private:
	WClock timer;
	Sampler*sampler;
	Sample2D BSDFSamples;
	Sample3D lightSamples;
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
	Vector3 integrateCore(Ray ray,unsigned int depth,int beginNode = 0);
};
