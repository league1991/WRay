#include "StdAfx.h"
#include "RecursivePathIntegrator.h"

WRecursivePathIntegrator::WRecursivePathIntegrator
(WScene *scene, WAccelerator *tree,
 unsigned int ipathDepth,unsigned int inBranch,
 WSampler::WSamplerType samplerType,float imultiplier):

WSurfaceIntegrator(scene,tree),Dlighting(scene,tree),
multiplier(imultiplier),nBranchs(inBranch),
lightSamples(1),BSDFSamples(1)

{
	maxDepth=max(1,ipathDepth);
	if(samplerType==WSampler::SAMPLER_RANDOM)
		sampler=new WRandomSampler;
	allocateSamples();
}
WRecursivePathIntegrator::~WRecursivePathIntegrator()
{
	clearSamples();
}

void WRecursivePathIntegrator::setPathMaxDepth(unsigned int idepth)
{
	maxDepth=max(1,idepth);
	clearSamples();
	allocateSamples();
//	computeSamples();
}
void WRecursivePathIntegrator::clearSamples()
{
	BSDFSamples.clear();
	lightSamples.clear();
}
void WRecursivePathIntegrator::allocateSamples()
{
	unsigned int N=(pow(float(nBranchs),int(maxDepth+1))-1)/(nBranchs-1);
	unsigned int nLightSamples=scene->getLightNum()*N;
	unsigned int nBSDFSamples=(N-1)*2;
	BSDFSamples.setSize(unsigned int(sqrt(float(nBSDFSamples))+1));
	lightSamples.setSize(unsigned int(sqrt(float(nLightSamples))+1));
	BSDFSamples.allocateSpace();
	lightSamples.allocateSpace();
}
void WRecursivePathIntegrator::computeSamples()
{
	//	sampler->setSeed(unsigned int(clock()));
	sampler->computeSamples(BSDFSamples);
	sampler->computeSamples(lightSamples);
}

WVector3 WRecursivePathIntegrator::integrateCore( WRay ray,unsigned int depth,int beginNode /*= 0*/ )
{
//	computeSamples();
// 	if(depth==maxDepth)
// 		return WVector3(0);
	WDifferentialGeometry DG;
	WVector3 totalLight(0),directLight,indirectLight(0);
	WVector3 ri,ro;
	float rayPDF;
	WBSDF*bsdf;
	WMaterial*mtl;
	int endNode = 0;
	if(tree->intersect(ray,DG,&endNode, beginNode))
	{
		scene->getNthMaterial(mtl,DG.mtlId);
		mtl->buildBSDF(DG,bsdf);
		ro=-1*ray.direction;
		//			timer.end();
		//计算直接光照
		directLight=Dlighting.sampleAllLights(
			bsdf,lightSamples,BSDFSamples,ro);
		//			timer.begin();
//		directLight.showCoords();
		//递归计算间接光照,发射多条光线
		float bsdfU,bsdfV;
		for(unsigned int nthBranch=0;nthBranch<nBranchs;nthBranch++)
		{
			if(depth+1==maxDepth)
				break;
	// 		timer.begin();
			BSDFSamples.get2D(bsdfU,bsdfV);
//			cout<<bsdfU<<bsdfV<<endl;
			bsdf->sampleRay(bsdfU,bsdfV,ri,ro,rayPDF);
			ray.point=DG.position;
			ray.direction=ri;
			ray.tMax=M_INF_BIG;
			//发射一条光线

			WVector3 L=integrateCore(ray,depth+1,endNode);
			//L.showCoords();
// 			WVector3 F=bsdf->evaluateFCos(ri,ro);
//			F.showCoords();
			indirectLight+=L*bsdf->evaluateFCos(ri,ro)/rayPDF*multiplier;
		}
//		indirectLight.showCoords();
		delete bsdf;
		indirectLight/=nBranchs;
		totalLight=directLight+indirectLight;
	}
//	totalLight.showCoords();
	return totalLight;
}
WVector3 WRecursivePathIntegrator::integrate(WRay&ray)
{
//	allocateSamples();
	computeSamples();
	return integrateCore(ray,0,0);
}
void WRecursivePathIntegrator::displayTime()
{
	timer.display();
}