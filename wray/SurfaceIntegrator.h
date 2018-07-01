#pragma once

class SurfaceIntegrator
{
public:
	SurfaceIntegrator(Scene*iscene,WAccelerator*itree):
	  scene(iscene),tree(itree), m_memoryPool(4,4*1024){};
	virtual ~SurfaceIntegrator(void);
	virtual Vector3 integrate(Ray&ray){return Vector3(0);}
	virtual void displayTime(){}
	// prepare samples for the following render pass
	virtual void initSamples(int nSampleGroup = 10){}

	void setPixelInfo(const Vector2i& pixelPos, int pixelSampleIdx, int numPixelSamples)
	{
		m_pixelPos = pixelPos;
		m_pixelSampleIdx = pixelSampleIdx;
		m_numPixelSamples = numPixelSamples;
	}
protected:
    RandomNumber m_rng;
	MemoryPool m_memoryPool;
	Vector2i m_pixelPos;
	int m_pixelSampleIdx;
	int m_numPixelSamples;
	Scene*scene;
	WAccelerator*tree;
};
