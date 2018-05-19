#include "StdAfx.h"
#include "DirectLighting.h"



WDirectLighting::~WDirectLighting(void)
{
}

bool WDirectLighting::isVisible(Vector3 pos1, Vector3 pos2, int* beginNode)
{
	Vector3 delta=pos2-pos1;
	float length=delta.length();
	delta.normalize();
	//注意：参数tMin（就是这里的1e-1）会影响阴影的质量
	//值太小的时候，由于计算误差，
	//在模型表面会出现一些斑点，
	//太大的时候会使得阴影不准确
	Ray r(pos1,delta,1e-2f,length - 1e-2f);
	int begNode = beginNode ? *beginNode : -1;
	return !tree->isIntersect(r, begNode);
}
Vector3 WDirectLighting::computeDirectLight(Light *light, BSDF *bsdf, Sample3D &lightSample, Sample2D &bsdfSample,const Vector3&ro, int* nodeInfo)
{
	// Sample from one light
	float LSu,LSv,LSw;
	lightSample.get3D(LSu,LSv,LSw);
	float lightPDF;
	Vector3 lightPosition, intensity;
	light->sampleLight(LSu,LSv,LSw,*bsdf,lightPosition,intensity,lightPDF, m_memoryPool);
	lightPDF /= scene->getLightNum();
	Vector3 lightRadiance(0.0);
	if (!intensity.isZero())
	{
		if (isVisible(bsdf->DG.position, lightPosition, nodeInfo))
		{
			Vector3 ri = lightPosition - bsdf->DG.position;
			ri.normalize();
			Vector3 fCos = bsdf->evaluateFCos(ri, ro);
			lightRadiance = fCos*intensity / lightPDF;
		}
		else
		{
			// LightPDF is measured in solid angle space
			// If the surface is occluded, that means we failed to sample from light
			lightPDF = 0;
		}
	}

	// Get bsdf sample ray
	float BSu, BSv;
	bsdfSample.get2D(BSu, BSv);
	Vector3 sampleWi;
	float bsdfPDF;
	bsdf->sampleRay(BSu, BSv, sampleWi, ro, bsdfPDF);

	Vector3 bsdfRadiance(0.0);
	Ray ray(bsdf->DG.position, sampleWi);
	DifferentialGeometry DG;
	int beginNode = 0, endNode;
	if (tree->intersect(ray, DG, &endNode, beginNode))
	{
		WMaterial*mtl;
		scene->getNthMaterial(mtl, DG.mtlId);
		BSDF* sourceBSDF;
		mtl->buildBSDF(DG, sourceBSDF, m_memoryPool);
		Vector3 emission = sourceBSDF->getEmission();
		if (!emission.isZero() && bsdfPDF > 0)
		{
			Vector3 fCos = bsdf->evaluateFCos(sampleWi, ro);
			bsdfRadiance = fCos*emission / bsdfPDF;
		}
		mtl->freeBSDF(sourceBSDF, m_memoryPool);
	}

	if (lightPDF <= 0 && bsdfPDF <= 0)
	{
		return Vector3(0.0);
	}
	float lightWeight = RandomNumber::powerHeuristic(1, lightPDF, 1, bsdfPDF);
	float bsdfWeight =  RandomNumber::powerHeuristic(1, bsdfPDF, 1, lightPDF);
	return lightRadiance * lightWeight + bsdfRadiance * bsdfWeight;
}

Vector3 WDirectLighting::sampleAllLights(BSDF *bsdf, Sample3D &lightSample, Sample2D &bsdfSample, const Vector3 &ro, int* nodeInfo)
{	
	Light*pLight;
	Vector3 color(0);
	unsigned int lightNum=scene->getLightNum();
	
	for (int ithLight = 0; ithLight < scene->getLightNum(); ithLight++)
	{
		pLight = scene->getLightPointer(ithLight);
		color += computeDirectLight(pLight, bsdf, lightSample, bsdfSample, ro, nodeInfo);
	}
	color /= scene->getLightNum();
	return color + bsdf->getEmission();
}

Vector3 WDirectLighting::sampleOneLight(BSDF *bsdf, Sample3D &lightSample, Sample2D &bsdfSample, const Vector3 &ro, int* nodeInfo)
{
	Light*pLight;
	Vector3 color(0);
	unsigned int lightNum = scene->getLightNum();

	int ithLight = RandomNumber::randomInt(lightNum);
	pLight = scene->getLightPointer(ithLight);
	color += computeDirectLight(pLight, bsdf, lightSample, bsdfSample, ro, nodeInfo);
	return color + bsdf->getEmission();
}
