#include "StdAfx.h"
#include "Material.h"



WMaterial::~WMaterial(void)
{
}

void WLambertMaterial::buildBSDF(DifferentialGeometry DG,BSDF*&bsdf)
{
	bsdf=new LambertBSDF(DG,color);
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
	bsdf=new PhongBSDF(DG,color,specular,glossiness);
	bsdf->setEmission(this->getLight());
}
void WPerfectReflectionMaterial::buildBSDF(DifferentialGeometry DG,BSDF*&bsdf)
{
	bsdf=new PerfectReflectionBSDF(DG,color);
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
	bsdf=new PerfectRefractionBSDF(DG,color,IOR);
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
	bsdf=new MetalBSDF(DG,eta,k,exp);
	bsdf->setEmission(this->getLight());
}
void WDielectricMaterial::buildBSDF(DifferentialGeometry DG,BSDF*&bsdf)
{
	bsdf=new DielectricBSDF(DG,ior,color,exp);
	bsdf->setEmission(this->getLight());
}
