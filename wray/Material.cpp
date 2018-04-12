#include "StdAfx.h"
#include "Material.h"



WMaterial::~WMaterial(void)
{
}

void WLambertMaterial::buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf)
{
	bsdf=new WLambertBSDF(DG,color);
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

void WPhongMaterial::buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf)
{
	bsdf=new WPhongBSDF(DG,color,specular,glossiness);
	bsdf->setEmission(this->getLight());
}
void WPerfectReflectionMaterial::buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf)
{
	bsdf=new WPerfectReflectionBSDF(DG,color);
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

void WPerfectRefractionMaterial::buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf)
{
	bsdf=new WPerfectRefractionBSDF(DG,color,IOR);
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
void WMetalMaterial::buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf)
{
	bsdf=new WMetalBSDF(DG,eta,k,exp);
	bsdf->setEmission(this->getLight());
}
void WDielectricMaterial::buildBSDF(WDifferentialGeometry DG,WBSDF*&bsdf)
{
	bsdf=new WDielectricBSDF(DG,ior,color,exp);
	bsdf->setEmission(this->getLight());
}
