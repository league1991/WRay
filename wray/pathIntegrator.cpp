#include "StdAfx.h"
#include "pathIntegrator.h"
#include "Scene.h"

PathIntegrator::PathIntegrator(Scene *scene, WAccelerator *tree, unsigned int ipathDepth, Sampler::SamplerType samplerType,float imultiplier):
WSurfaceIntegrator(scene,tree),Dlighting(scene,tree),
multiplier(imultiplier),
lightSamples(4),BSDFSamples(4)
{
	pathMaxDepth=max(1,ipathDepth);
	if(samplerType==Sampler::SAMPLER_RANDOM)
		sampler=new RandomSampler;
	else if(samplerType==Sampler::SAMPLER_STRATIFIED)
		sampler=new StratifiedSampler;
	else if (samplerType == Sampler::SAMPLER_SEQUENCE_STRATIFIED)
	{
		sampler = new SequenceStratifiedSampler;
	}
	allocateSamples();
}
PathIntegrator::~PathIntegrator()
{
	clearSamples();
}

void PathIntegrator::setPathMaxDepth(unsigned int idepth)
{
	pathMaxDepth=max(1,idepth);
	clearSamples();
	allocateSamples();
}
void PathIntegrator::clearSamples()
{
	BSDFSamples.clear();
	lightSamples.clear();
}
void PathIntegrator::allocateSamples()
{
	unsigned int nLightSamples=scene->getLightNum()*pathMaxDepth;
	unsigned int nBSDFSamples=pathMaxDepth*2;
	BSDFSamples.setSize(nBSDFSamples);
	lightSamples.setSize(unsigned int(sqrt(float(nLightSamples))+1));
	BSDFSamples.allocateSpace();
	lightSamples.allocateSpace();
}
void PathIntegrator::computeSamples()
{
//	sampler->setSeed(unsigned int(clock()));
	if (sampler->type == Sampler::SAMPLER_SEQUENCE_STRATIFIED)
	{
		SequenceStratifiedSampler* seqSampler = static_cast<SequenceStratifiedSampler*>(sampler);
		seqSampler->setPixel(m_pixelPos.m_x, m_pixelPos.m_y);
		seqSampler->setSampleIdx(m_pixelSampleIdx);
		seqSampler->setDimension((int)sqrt(float(m_numPixelSamples)));
	}
	sampler->computeSamples(BSDFSamples);
	sampler->computeSamples(lightSamples);
}

Vector3 PathIntegrator::integrate(Ray&camRay)//ÑÕÉ«¼ÆËã
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
	Ray ray=camRay;
//				cout<<ray.tMin<<endl;
//	ray.tMin=1e-5f;
	int beginNode = -1, endNode = -1;
	for(unsigned int depth=0; depth<pathMaxDepth;depth++)
	{
		if(tree->intersect(ray,DG,&endNode,beginNode))
		{
			if(depth==0)
				camRay.tMax=ray.tMax;
			WMaterial*mtl;
			scene->getNthMaterial(mtl,DG.mtlId);

			WBSDF*bsdf;
			mtl->buildBSDF(DG,bsdf);
			std::unique_ptr<WBSDF> bsdfPtr(bsdf);

			ro=-1*ray.direction;
			directLight=Dlighting.sampleAllLights(bsdf,lightSamples,BSDFSamples,ro, &endNode);
			totalLight+=pathThroughPut*directLight;
			float bsdfU,bsdfV;
			BSDFSamples.get2D(bsdfU,bsdfV);
			bsdf->sampleRay(bsdfU, bsdfV,ri,ro,rayPDF);
			ray.point=DG.position;
			ray.direction=ri;
			ray.tMin = 0.01f;
			ray.tMax=M_INF_BIG;
			pathThroughPut*=bsdf->evaluateFCos(ri,ro)/rayPDF;
			beginNode = endNode;
		}
		else 
			break;
		if (depth > 3)
		{
			float continueProb = 0.7;
			if (RandomNumber::randomFloat() > continueProb)
				break;
			pathThroughPut/=continueProb;
		}
	}
	//	totalLight.showCoords();
	totalLight.x = min(totalLight.x,5.f);
	totalLight.y = min(totalLight.y,5.f);
	totalLight.z = min(totalLight.z,5.f);
	return totalLight;
}
void PathIntegrator::displayTime()
{
	timer.display();
}

void PathIntegrator::initSamples( int nSampleGroup )
{
}
