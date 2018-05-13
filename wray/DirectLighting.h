#pragma once
#include "surfaceintegrator.h"

class WDirectLighting :
	public WSurfaceIntegrator
{
public:
	WDirectLighting(Scene *iscene,WAccelerator*itree):
	  WSurfaceIntegrator(iscene,itree){}
	virtual ~WDirectLighting(void);

	//计算场景所有灯光的光照
	Vector3 sampleAllLights(
		BSDF*bsdf,Sample3D&lightSample,
		Sample2D&bsdfSample,const Vector3&ro, int* nodeInfo = NULL);

	Vector3 sampleOneLight(
		BSDF*bsdf, Sample3D&lightSample,
		Sample2D&bsdfSample, const Vector3&ro, int* nodeInfo = NULL);
private:
	//计算选定的灯光所产生的直接光照
	//ro为光线出射方向
	Vector3 computeDirectLight(
		Light *light,BSDF*bsdf,Sample3D&lightSample,
		Sample2D&bsdfSample,const Vector3&ro, int* nodeInfo = NULL);

	bool isVisible(Vector3 pos1,Vector3 pos2, int* beginNode = NULL);

};
