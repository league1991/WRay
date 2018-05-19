#include "StdAfx.h"
#include "Material.h"


MemoryPool WMaterial::s_bsdfPool(4,4*1024);

WMaterial::~WMaterial(void)
{
}

void WLambertMaterial::buildBSDF(DifferentialGeometry DG,BSDF*&bsdf)
{
	void* buffer = s_bsdfPool.allocate<LambertBSDF>();
	bsdf=new (buffer)LambertBSDF(DG,color);
	bsdf->setEmission(this->getLight());
}

void WLambertMaterial::getProperties( vector<float>& properties )
{
	properties.clear();
	properties.push_back(color.x);
	properties.push_back(color.y);
	properties.push_back(color.z);
	properties.push_back(1.0f);
}

void WPhongMaterial::buildBSDF(DifferentialGeometry DG,BSDF*&bsdf)
{
	void* buffer = s_bsdfPool.allocate<PhongBSDF>();
	bsdf=new (buffer)PhongBSDF(DG,color,specular,glossiness);
	bsdf->setEmission(this->getLight());
}
void WPerfectReflectionMaterial::buildBSDF(DifferentialGeometry DG,BSDF*&bsdf)
{
	void* buffer = s_bsdfPool.allocate<PerfectReflectionBSDF>();
	bsdf=new (buffer)PerfectReflectionBSDF(DG,color);
	bsdf->setEmission(this->getLight());
}

void WPerfectReflectionMaterial::getProperties( vector<float>& properties )
{
	properties.clear();
	properties.push_back(color.x);
	properties.push_back(color.y);
	properties.push_back(color.z);
	properties.push_back(1.0f);
}

void WPerfectRefractionMaterial::buildBSDF(DifferentialGeometry DG,BSDF*&bsdf)
{
	void* buffer = s_bsdfPool.allocate<PerfectRefractionBSDF>();
	bsdf=new (buffer)PerfectRefractionBSDF(DG,color,IOR);
	bsdf->setEmission(this->getLight());
}

void WPerfectRefractionMaterial::getProperties( vector<float>& properties )
{
	properties.clear();
	properties.push_back(color.x);
	properties.push_back(color.y);
	properties.push_back(color.z);
	properties.push_back(IOR);
}

WMetalMaterial::WMetalMaterial(string iName,unsigned int iID,Vector3 Fr,float iexp, Vector3 ilight):WMaterial(MATERIAL_METAL,iName,iID,Fr,ilight),exp(iexp)
{
	k=2.0f*(color/(Vector3(1)-color)).sqrtElement();
	Vector3 colorSqrt=color.sqrtElement();
	eta=(Vector3(1)+colorSqrt)/(Vector3(1)-colorSqrt);
// 	k.showCoords();
// 	eta.showCoords();
}
void WMetalMaterial::refreshColor()
{
	k=2.0f*(color/(Vector3(1)-color)).sqrtElement();
	Vector3 colorSqrt=color.sqrtElement();
	eta=(Vector3(1)+colorSqrt)/(Vector3(1)-colorSqrt);
}
void WMetalMaterial::buildBSDF(DifferentialGeometry DG,BSDF*&bsdf)
{
	void* buffer = s_bsdfPool.allocate<MetalBSDF>();
	bsdf=new (buffer)MetalBSDF(DG,eta,k,exp);
	bsdf->setEmission(this->getLight());
}
void WDielectricMaterial::buildBSDF(DifferentialGeometry DG,BSDF*&bsdf)
{
	void* buffer = s_bsdfPool.allocate<DielectricBSDF>();
	bsdf=new (buffer)DielectricBSDF(DG,ior,color,exp);
	bsdf->setEmission(this->getLight());
}
