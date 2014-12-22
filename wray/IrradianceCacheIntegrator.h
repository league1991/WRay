#pragma once
#include "surfaceintegrator.h"

class WIrradianceCacheIntegrator :
	public WSurfaceIntegrator
{
public:
	WIrradianceCacheIntegrator(
		WScene*iscene,WAccelerator*itree,
		unsigned int ioctTreeMaxDepth=10,
		unsigned int inRaysForLambert=50,
		unsigned int inRaysForSpecular=10,
		unsigned int iminSamples=10,
		unsigned int inDirectLightSamples=1,
		unsigned int imaxTracingDepth=5,
		WSampler::WSamplerType samplerType=
		WSampler::SAMPLER_RANDOM,
		float imaxNormalError=0.1,
		float imaxPlanarError=0.1,
		float idistanceErrorFactor=0.5,
		float ismoothFactor=2.0
		);

	virtual ~WIrradianceCacheIntegrator(void);

	WVector3 integrate(WRay&ray);
	//设置插值器参数
	void setInterpolatorParams(
		float imaxNormalError=0.1,
		float imaxPlanarError=0.1,
		float ismoothFactor=2.0,
		unsigned int iminSamples=10);

	void setOctTreeParams(unsigned int imaxDepth)
	{octTree.setMaxDepth(imaxDepth);}

	void setSampleParams(
		unsigned int inRaysForLambert=50,
		unsigned int inRaysForSpecular=10,
		unsigned int inDirectLightSamples=1,
		float idistanceErrorFactor=0.4
		);

	void pathTracingParams(unsigned int imaxDepth)
	{maxTracingDepth=imaxDepth;}

	void clearTree();
	void displaySamplePoints(bool isDisplayBox=false);
	void displayTreeNodes();

private:
	WSampler*sampler;
 	WSample2D LambertBSDFSamples;
	WSample2D SpecularBSDFSamples;
 	WSample3D lightSamples;
	WDirectLighting Dlighting;
	WPathIntegrator pathIntegrator;
	//插值器
	WIrradianceInterpolator interpolator;
	WOctTree octTree;

	unsigned int nRaysForLambert;
	unsigned int nRaysForSpecular;
	unsigned int maxTracingDepth;
	unsigned int nDirectLightSamples;
	//样本包围盒的大小与追踪光线的平均距离成正比
	//此为比例系数
	float distanceErrorFactor;


	//通过对现有采样点插值求出E
	//返回布尔值表示插值是否成功
	//point和normal分别表示要进行插值的点的位置和法线方向
	bool interpolate(
		WVector3&point,
		WVector3&normal,
		WVector3&E);

	//计算新的采样点，并把它加入八叉树
	//返回E值
	WVector3 computeNewSamples(WBSDF*bsdf);

	//计算间接光照，返回L值
	WVector3 computeIndirectLight(WBSDF*bsdf);

	//采用路径跟踪办法计算，直到达最大路径深度
	//或者路径遇到漫反射表面的时候停止,返回L值
	void pathTracing(WRay&ray,
		WVector3&indirectLight,WVector3&pathThroughPut);


};
