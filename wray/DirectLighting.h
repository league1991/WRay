#pragma once
#include "surfaceintegrator.h"

class WDirectLighting :
	public WSurfaceIntegrator
{
public:
	WDirectLighting(WScene *iscene,WAccelerator*itree):
	  WSurfaceIntegrator(iscene,itree){}
	virtual ~WDirectLighting(void);

	//计算场景所有灯光的光照
	WVector3 sampleAllLights(
		WBSDF*bsdf,WSample3D&lightSample,
		WSample2D&bsdfSample,const WVector3&ro, int* nodeInfo = NULL);
	
private:
	//计算选定的灯光所产生的直接光照
	//ro为光线出射方向
	WVector3 computeDirectLight(
		WLight *light,WBSDF*bsdf,WSample3D&lightSample,
		WSample2D&bsdfSample,const WVector3&ro, int* nodeInfo = NULL);

	bool isVisible(WVector3 pos1,WVector3 pos2, int* beginNode = NULL);

};
