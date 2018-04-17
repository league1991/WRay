#pragma once

class WSurfaceIntegrator
{
public:
	WSurfaceIntegrator(Scene*iscene,WAccelerator*itree):
	  scene(iscene),tree(itree){};
	virtual ~WSurfaceIntegrator(void);
	virtual Vector3 integrate(WRay&ray){return Vector3(0);}
	virtual void displayTime(){}

	// prepare samples for the following render pass
	virtual void initSamples(int nSampleGroup = 10){}
protected:
	Scene*scene;
	WAccelerator*tree;
};
