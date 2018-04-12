#include "StdAfx.h"
#include "pathIntegrator.h"
#include "Scene.h"

WPathIntegrator::WPathIntegrator(WScene *scene, WAccelerator *tree, unsigned int ipathDepth, WSampler::WSamplerType samplerType,float imultiplier):
WSurfaceIntegrator(scene,tree),Dlighting(scene,tree),
multiplier(imultiplier),
lightSamples(4),BSDFSamples(4)
{
	pathMaxDepth=max(1,ipathDepth);
	if(samplerType==WSampler::SAMPLER_RANDOM)
		sampler=new WRandomSampler;
	else if(samplerType==WSampler::SAMPLER_STRATIFIED)
		sampler=new WStratifiedSampler;
	allocateSamples();
}
WPathIntegrator::~WPathIntegrator()
{
	clearSamples();
}

void WPathIntegrator::setPathMaxDepth(unsigned int idepth)
{
	pathMaxDepth=max(1,idepth);
	clearSamples();
	allocateSamples();
}
void WPathIntegrator::clearSamples()
{
	BSDFSamples.clear();
	lightSamples.clear();
}
void WPathIntegrator::allocateSamples()
{
	unsigned int nLightSamples=scene->getLightNum()*pathMaxDepth;
	unsigned int nBSDFSamples=pathMaxDepth*2;
	BSDFSamples.setSize(unsigned int(sqrt(float(nBSDFSamples))+1));
	lightSamples.setSize(unsigned int(sqrt(float(nLightSamples))+1));
	BSDFSamples.allocateSpace();
	lightSamples.allocateSpace();
}
void WPathIntegrator::computeSamples()
{
//	sampler->setSeed(unsigned int(clock()));
	sampler->computeSamples(BSDFSamples);
	sampler->computeSamples(lightSamples);
}

Vector3 WPathIntegrator::integrate(WRay&camRay)//ÑÕÉ«¼ÆËã
{
// 	clearSamples();
// 	allocateSamples();
	computeSamples();
//	BSDFSamples.display();
//	cout<<"begin"<<endl;
	WDifferentialGeometry DG;
	Vector3 pathThroughPut(1.0f);
	Vector3 totalLight(0),directLight;
	Vector3 ri,ro;
	float rayPDF;
	WBSDF*bsdf;
	WRay ray=camRay;
//				cout<<ray.tMin<<endl;
//	ray.tMin=1e-5f;
	int beginNode = -1, endNode = -1;
	for(unsigned int depth=0;depth<pathMaxDepth;depth++)
	{
		if(tree->intersect(ray,DG,&endNode,beginNode))
		{
			if(depth==0)
				camRay.tMax=ray.tMax;
			WMaterial*mtl;
			scene->getNthMaterial(mtl,DG.mtlId);

			mtl->buildBSDF(DG,bsdf);
			ro=-1*ray.direction;
			directLight=Dlighting.sampleAllLights(
				bsdf,lightSamples,BSDFSamples,ro, &endNode);
			totalLight+=pathThroughPut*directLight;
			float bsdfU,bsdfV;
			BSDFSamples.get2D(bsdfU,bsdfV);
			bsdf->sampleRay(bsdfU, bsdfV,ri,ro,rayPDF);
			ray.point=DG.position;
			ray.direction=ri;
			ray.tMin = 0.01f;
			ray.tMax=M_INF_BIG;
			pathThroughPut*=bsdf->evaluateFCos(ri,ro)/rayPDF;
			delete bsdf;
			beginNode = endNode;
		}
		else 
			break;
		if (depth > 3)
		{
			float continueProb = 0.7;
			if (WMonteCarlo::randomFloat() > continueProb)
				break;
			pathThroughPut/=continueProb;
		}
	}
	//	totalLight.showCoords();
	//totalLight.x = min(totalLight.x,5.f);
	//totalLight.y = min(totalLight.y,5.f);
	//totalLight.z = min(totalLight.z,5.f);
	return totalLight;
}
void WPathIntegrator::displayTime()
{
	timer.display();
}

void WPathIntegrator::initSamples( int nSampleGroup )
{
}
