#include "StdAfx.h"
#include "IrradianceCacheIntegrator.h"

WIrradianceCacheIntegrator::WIrradianceCacheIntegrator(
	WScene*iscene,WAccelerator*itree,
	unsigned int ioctTreeMaxDepth,
	unsigned int inRaysForLambert,
	unsigned int inRaysForSpecular,
	unsigned int iminSamples,
	unsigned int inDirectLightSamples,
	unsigned int imaxTracingDepth,
	WSampler::WSamplerType samplerType,
	float imaxNormalError,
	float imaxPlanarError,
	float idistanceErrorFactor,
	float ismoothFactor):

	WSurfaceIntegrator(iscene,itree),
	interpolator(
	imaxNormalError,imaxPlanarError,ismoothFactor,iminSamples),
	octTree(iscene,ioctTreeMaxDepth),
	pathIntegrator(iscene,itree,1,samplerType,1.0),
	Dlighting(iscene,itree),
	nRaysForLambert(inRaysForLambert),
	nRaysForSpecular(inRaysForSpecular),
	nDirectLightSamples(inDirectLightSamples),
	maxTracingDepth(imaxTracingDepth),
	LambertBSDFSamples(1),SpecularBSDFSamples(1),lightSamples(1),
	distanceErrorFactor(idistanceErrorFactor)
{
	if(samplerType==WSampler::SAMPLER_RANDOM)
		sampler=new WRandomSampler;
	else if(samplerType==WSampler::SAMPLER_STRATIFIED)
		sampler=new WStratifiedSampler;

	LambertBSDFSamples.setSize(sqrt(float(nRaysForLambert))+1);
	LambertBSDFSamples.allocateSpace();
	SpecularBSDFSamples.setSize(sqrt(float(nRaysForSpecular))+1);
	SpecularBSDFSamples.allocateSpace();

	unsigned int nLightSamples=
		scene->getLightNum()*nDirectLightSamples;
	lightSamples.setSize(sqrt(float(nLightSamples))+1);
	lightSamples.allocateSpace();
}

WIrradianceCacheIntegrator::~WIrradianceCacheIntegrator(void)
{
	octTree.clear();
	LambertBSDFSamples.clear();
	SpecularBSDFSamples.clear();
	lightSamples.clear();
}
void WIrradianceCacheIntegrator::setInterpolatorParams(
	float imaxNormalError,
	float imaxPlanarError ,
	float ismoothFactor,
	unsigned int iminSamples)
{
	interpolator.setInterpolateParams(
		imaxNormalError,
		imaxPlanarError,
		iminSamples,
		ismoothFactor);
}
void WIrradianceCacheIntegrator::setSampleParams(
	unsigned int inRaysForLambert,
	unsigned int inRaysForSpecular,
	unsigned int inDirectLightSamples,
	float idistanceErrorFactor)
{
	nRaysForLambert=inRaysForLambert;
	nRaysForSpecular=inRaysForSpecular;
	nDirectLightSamples=inDirectLightSamples;

	LambertBSDFSamples.setSize(sqrt(float(nRaysForLambert))+1);
	LambertBSDFSamples.allocateSpace();
	SpecularBSDFSamples.setSize(sqrt(float(nRaysForSpecular))+1);
	SpecularBSDFSamples.allocateSpace();
	distanceErrorFactor=idistanceErrorFactor;

	unsigned int nLightSamples=
		scene->getLightNum()*nDirectLightSamples;
	lightSamples.setSize(sqrt(float(nLightSamples))+1);
	lightSamples.allocateSpace();
}
void WIrradianceCacheIntegrator::displaySamplePoints(bool isDisplayBox)
{
	octTree.displaySamples(isDisplayBox);
}
void WIrradianceCacheIntegrator::clearTree()
{
	octTree.clear();
}

WVector3 WIrradianceCacheIntegrator::integrate(WRay&ray)
{
	WDifferentialGeometry DG;
	WBSDF*bsdf=NULL;
	WVector3 ro,directLight=WVector3(0),indirectLight=WVector3(0);
	int beginNode = 0, endNode;
	if(tree->intersect(ray,DG,&endNode,beginNode))
	{
		//由材质生成BSDF
		WMaterial*mtl;
		scene->getNthMaterial(mtl,DG.mtlId);
		mtl->buildBSDF(DG,bsdf);
		sampler->computeSamples(LambertBSDFSamples);
		sampler->computeSamples(lightSamples);

		//计算直接光照
		ro=-1*ray.direction;
		for(unsigned int i=0;i<nDirectLightSamples;i++)
		{
			directLight=Dlighting.sampleAllLights(
				bsdf,lightSamples,LambertBSDFSamples,ro);
		}
		directLight/=nDirectLightSamples;

//		directLight.showCoords();
		//对于镜面反射和折射，直接用路径跟踪办法
		if(!(bsdf->type==WBSDF::BSDF_LAMBERT||
			bsdf->type==WBSDF::BSDF_PHONG))
		{
//				return WVector3(0);
			float bsdfU,bsdfV;
			unsigned int nIntersectedRays=0;
			WRay ray;				//需要再次发射的光线
			WVector3 ri,ro,L;
			float PDF;
			sampler->computeSamples(SpecularBSDFSamples);

			//对于完全镜面反射，采用路径跟踪办法
			for(unsigned int i=0;i<nRaysForSpecular;i++)
			{
				SpecularBSDFSamples.get2D(bsdfU,bsdfV);
				ro=bsdf->DG.rayDir;

				//发射一条光线
				bsdf->sampleRay(bsdfU,bsdfV,ri,ro,PDF);
				ray.point=bsdf->DG.position;
				ray.direction=ri;
				ray.tMax=M_INF_BIG;
				ray.tMin=1e-5f;
				//发射方向与当前法线夹角余弦绝对值
//				cosThetaAbs=abs(bsdf->DG.normal.dot(ri));

				//对每一条光线进行路径跟踪，直到到达
				//漫反射表面为止
				WVector3 pathDirectLight;
				WVector3 pathThroughPut=
					bsdf->evaluateFCos(ri,ro)/PDF;
//				WBSDF*pathBSDF;
				WDifferentialGeometry pathDG;
//				Material*pathMtl;
//				float pathBSDFU,pathBSDFV;
				bool isGetLambert=false;
				unsigned int depth=maxTracingDepth;

				
// 				while(depth--)
// 				{
// 					//把之前发射的光线与场景求交，
// 					//如果交点为漫反射表面
// 					//停止路径跟踪
// 					if(tree->intersect(ray,pathDG))
// 					{
// 						//找到交点的材质和BSDF
// 						scene->getNthMaterial(pathMtl,pathDG.mtlId);
// 						pathMtl->buildBSDF(pathDG,pathBSDF);
// 						ro=-1*ray.direction;
// 
// 						//如果发射的光线遇到漫反射表面
// 						//停止路径跟踪，用irradianceCach
// 						//方法估算直接光照
// 						if(pathBSDF->type==WBSDF::BSDF_LAMBERT||
// 							pathBSDF->type==WBSDF::BSDF_PHONG)
// 						{
// 							isGetLambert=true;
// 							break;
// 						}
// 
// 						pathDirectLight=Dlighting.sampleAllLights(
// 							pathBSDF,lightSamples,LambertBSDFSamples,ro);
// 						indirectLight+=pathThroughPut*pathDirectLight;
// 						
// 						pathBSDFU=WMonteCarlo::randomFloat();
// 						pathBSDFV=WMonteCarlo::randomFloat();
// 						pathBSDF->sampleRay(pathBSDFU,pathBSDFV,ri,ro,PDF);
// 						ray.point=pathDG.position;
// 						ray.direction=ri;
// 						ray.tMax=M_INF_BIG;
// 
// 						pathThroughPut*=pathBSDF->evaluateFCos(ri,ro)/PDF;
// 						delete pathBSDF;
// 					}
// 					else 
// 					{
// 						break;
// 					}
// 				}
// 
// 
// 				
// 
// 				if(isGetLambert)
// 				{
// 					//如果是因为最终遇到漫反射表面而停止跟踪的
// 					//用irradianceCach方法估算漫反射表面发出的光线
// 					pathDirectLight=computeIndirectLight(pathBSDF);
// 					indirectLight+=pathThroughPut*pathDirectLight;
// 				}
 				pathTracing(ray,indirectLight,pathThroughPut);

			}
//			directLight.showCoords();
			indirectLight/=nRaysForSpecular;
//			indirectLight.showCoords();
//			cout<<"a"<<endl;
			return directLight+indirectLight;
		}
		//对于漫反射，用irradianceCache办法解决
		else
		{
//			return WVector3(0);
			indirectLight+=computeIndirectLight(bsdf);
			delete bsdf;
			return directLight+indirectLight;
		}
	}
	delete bsdf;
	return WVector3(0);
}


void WIrradianceCacheIntegrator::pathTracing(
	WRay&ray, 
	WVector3&indirectLight,WVector3&pathThroughPut)
{
	WVector3 pathDirectLight;
	WVector3 ri,ro;
	WBSDF*pathBSDF;
	WDifferentialGeometry pathDG;
	WMaterial*pathMtl;
	float pathBSDFU,pathBSDFV;
	unsigned int depth=maxTracingDepth;
	float PDF;
	bool isGetLambert=false;
	int beginNode = 0, endNode = 0;
	while(depth--)
	{
		//把之前发射的光线与场景求交，
		//如果交点为漫反射表面
		//停止路径跟踪
		if(tree->intersect(ray,pathDG, &endNode, beginNode))
		{
			//找到交点的材质和BSDF
			scene->getNthMaterial(pathMtl,pathDG.mtlId);
			pathMtl->buildBSDF(pathDG,pathBSDF);
			ro=-1*ray.direction;

			//如果发射的光线遇到漫反射表面
			//停止路径跟踪，用irradianceCach
			//方法估算直接光照
			if(pathBSDF->type==WBSDF::BSDF_LAMBERT||
				pathBSDF->type==WBSDF::BSDF_PHONG)
			{
				isGetLambert=true;
//				delete pathBSDF;
				break;
			}

			pathDirectLight=Dlighting.sampleAllLights(
				pathBSDF,lightSamples,LambertBSDFSamples,ro);
			indirectLight+=pathThroughPut*pathDirectLight;

			pathBSDFU=WMonteCarlo::randomFloat();
			pathBSDFV=WMonteCarlo::randomFloat();
			pathBSDF->sampleRay(pathBSDFU,pathBSDFV,ri,ro,PDF);
			ray.point=pathDG.position;
			ray.direction=ri;
			ray.tMax=M_INF_BIG;

			pathThroughPut*=pathBSDF->evaluateFCos(ri,ro)/PDF;
			delete pathBSDF;	
			beginNode = endNode;
		}
		else 
		{

			break;
		}
	}
	if(isGetLambert)
	{
		//如果是因为最终遇到漫反射表面而停止跟踪的
		//用irradianceCach方法估算漫反射表面发出的光线
		pathDirectLight=computeIndirectLight(pathBSDF);
		indirectLight+=pathThroughPut*pathDirectLight;
		delete pathBSDF;
	}
}

bool WIrradianceCacheIntegrator::interpolate(
	WVector3&point,
	WVector3&normal,
	WVector3&E)
{
	//初始化结构
	interpolator.refresh(point,normal);
	//找到相邻的采样点，并尝试进行插值，实际上是求出加权和
	octTree.process(interpolator);
	//检测插值是否成功，如成功，返回true，并求出加权平均
	//否则返回false
	return interpolator.finalInterpolate(E);
}
WVector3 WIrradianceCacheIntegrator::computeNewSamples(WBSDF*bsdf)
{
	float bsdfU,bsdfV;
	float rayTotalLength=0;
//	float rayMinLength=M_INF_BIG;
	float cosThetaAbs;
	unsigned int nIntersectedRays=0;
	sampler->computeSamples(LambertBSDFSamples);
	WVector3 L(0);
	WVector3 Li;
	WDifferentialGeometry DG;
	WRay ray;
	WVector3 ri,ro;
	ro=bsdf->DG.rayDir;
	WVector3 directLight;
	float PDF;

	//发射nSecondaryRays条光线
	for(unsigned int i=0;i<nRaysForLambert;i++)
	{
		LambertBSDFSamples.get2D(bsdfU,bsdfV);


		//对光线采样
		bsdf->sampleRay(bsdfU,bsdfV,ri,ro,PDF);
		ray.point=bsdf->DG.position;
		ray.direction=ri;
		ray.tMax=M_INF_BIG;
		ray.tMin=1e-5f;
		//发射方向与当前法线夹角余弦绝对值
		cosThetaAbs=max(bsdf->DG.normal.dot(ri),1e-3f);
//		cout<<PDF<<endl;

		//只计算直接光照
		pathIntegrator.setPathMaxDepth(3);
		L+=pathIntegrator.integrate(ray)*cosThetaAbs/max(1e-2f, PDF);
		if(ray.tMax!=M_INF_BIG)
		{
			nIntersectedRays++;
			rayTotalLength+=ray.tMax;
// 			if(ray.tMax<rayMinLength)
// 				rayMinLength=ray.tMax;
		}
	}
//	L.showCoords();
	WVector3 E;
	if(nIntersectedRays>0)
	{
		E=L/(float)nIntersectedRays;
	}
	else
		E=WVector3(0);
//	cout<<nIntersectedRays<<endl;
//	E.showCoords();
	WIrradianceSample irSample;
	irSample.E=E;
	irSample.maxDist=
		rayTotalLength/nIntersectedRays*
		distanceErrorFactor;//*E.lengthSquared();
//	cout<<10000000000000000<<irSample.maxDist<<endl;
//	cout<<nIntersectedRays;
	irSample.normal=bsdf->DG.normal;
	irSample.point=bsdf->DG.position;
	octTree.addSample(irSample);
//	E.showCoords();
	return E;
}

WVector3 WIrradianceCacheIntegrator::computeIndirectLight(WBSDF *bsdf)
{
	WVector3 E;
	if(!interpolate(bsdf->DG.position,bsdf->DG.normal,E))
	{
//		cout<<"directcompute"<<endl;
		//在计算过程中会使用pathIntegrator
		//把其最大深度设为1，只计算光线遇到的物体的直接光照
		pathIntegrator.setPathMaxDepth(5);
		E=computeNewSamples(bsdf);
	}
	else
	{
//		cout<<"interpolate"<<endl;
	}
	return E*bsdf->rho(bsdf->DG.rayDir)/**M_PI*/;
}
void WIrradianceCacheIntegrator::displayTreeNodes()
{
	octTree.displayNodes();
}