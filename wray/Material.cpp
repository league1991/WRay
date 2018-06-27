#include "StdAfx.h"
#include "Material.h"

Material::~Material(void)
{
}

void LambertMaterial::buildBSDF(DifferentialGeometry DG,BSDF*&bsdf, MemoryPool& customPool)
{
	void* buffer = customPool.allocate<LambertBSDF>();
	bsdf=new (buffer)LambertBSDF(DG,color, &customPool);
	bsdf->setEmission(this->getLight());
}

void LambertMaterial::getProperties( vector<float>& properties )
{
	properties.clear();
	properties.push_back(color.x);
	properties.push_back(color.y);
	properties.push_back(color.z);
	properties.push_back(1.0f);
}

void PhongMaterial::buildBSDF(DifferentialGeometry DG,BSDF*&bsdf, MemoryPool& customPool)
{
	void* buffer = customPool.allocate<PhongBSDF>();
	bsdf=new (buffer)PhongBSDF(DG,color,specular,glossiness, &customPool);
	bsdf->setEmission(this->getLight());
}
void PerfectReflectionMaterial::buildBSDF(DifferentialGeometry DG,BSDF*&bsdf, MemoryPool& customPool)
{
	void* buffer = customPool.allocate<PerfectReflectionBSDF>();
	bsdf=new (buffer)PerfectReflectionBSDF(DG,color, &customPool);
	bsdf->setEmission(this->getLight());
}

void PerfectReflectionMaterial::getProperties( vector<float>& properties )
{
	properties.clear();
	properties.push_back(color.x);
	properties.push_back(color.y);
	properties.push_back(color.z);
	properties.push_back(1.0f);
}

void PerfectRefractionMaterial::buildBSDF(DifferentialGeometry DG,BSDF*&bsdf, MemoryPool& customPool)
{
	void* buffer = customPool.allocate<PerfectRefractionBSDF>();
	bsdf=new (buffer)PerfectRefractionBSDF(DG,color,IOR, &customPool);
	bsdf->setEmission(this->getLight());
}

void PerfectRefractionMaterial::getProperties( vector<float>& properties )
{
	properties.clear();
	properties.push_back(color.x);
	properties.push_back(color.y);
	properties.push_back(color.z);
	properties.push_back(IOR);
}

MetalMaterial::MetalMaterial(string iName,unsigned int iID,Vector3 Fr,float iexp, Vector3 ilight):Material(MATERIAL_METAL,iName,iID,Fr,ilight),exp(iexp)
{
	k=2.0f*(color/(Vector3(1)-color)).sqrtElement();
	Vector3 colorSqrt=color.sqrtElement();
	eta=(Vector3(1)+colorSqrt)/(Vector3(1)-colorSqrt);
}
void MetalMaterial::refreshColor()
{
	k=2.0f*(color/(Vector3(1)-color)).sqrtElement();
	Vector3 colorSqrt=color.sqrtElement();
	eta=(Vector3(1)+colorSqrt)/(Vector3(1)-colorSqrt);
}
void MetalMaterial::buildBSDF(DifferentialGeometry DG,BSDF*&bsdf, MemoryPool& customPool)
{
	void* buffer = customPool.allocate<MetalBSDF>();
	bsdf=new (buffer)MetalBSDF(DG,eta,k,exp, &customPool);
	bsdf->setEmission(this->getLight());
}
void DielectricMaterial::buildBSDF(DifferentialGeometry DG,BSDF*&bsdf, MemoryPool& customPool)
{
	void* buffer = customPool.allocate<DielectricBSDF>();
	bsdf=new (buffer)DielectricBSDF(DG,ior,color,exp, &customPool);
	bsdf->setEmission(this->getLight());
}

GGXMetalMaterial::GGXMetalMaterial(string iName, unsigned int iID, Vector3 Fr, float iexp, Vector3 ilight) :
	Material(MATERIAL_GGX_METAL, iName, iID, Fr, ilight), exp(iexp)
{
	refreshColor();
}

void GGXMetalMaterial::buildBSDF(DifferentialGeometry DG, BSDF *& bsdf, MemoryPool & customPool)
{
	void* buffer = customPool.allocate<GGXMetalBSDF>();
	bsdf = new (buffer)GGXMetalBSDF(DG, eta, k, exp, &customPool);
	bsdf->setEmission(this->getLight());
}

void GGXMetalMaterial::refreshColor()
{
	color.x = max(0.f, min(color.x, 0.9999f));
	color.y = max(0.f, min(color.y, 0.9999f));
	color.z = max(0.f, min(color.z, 0.9999f));
	k = 2.0f*(color / (Vector3(1) - color)).sqrtElement();
	Vector3 colorSqrt = color.sqrtElement();
	eta = (Vector3(1) + colorSqrt) / (Vector3(1) - colorSqrt);
}

void GGXMetalMaterial::setRoughness(float roughness)
{
	exp = roughness;
}

void GGXOpaqueMaterial::buildBSDF(DifferentialGeometry DG, BSDF *& bsdf, MemoryPool & customPool)
{
    void* buffer = customPool.allocate<GGXOpaqueBSDF>();
    bsdf = new (buffer)GGXOpaqueBSDF(DG, m_ior, color, m_ag, &customPool);
    bsdf->setEmission(this->getLight());
}

void GGXOpaqueMaterial::setRoughness(float roughness)
{
    m_ag = roughness;
}

void GGXTransparentMaterial::buildBSDF(DifferentialGeometry DG, BSDF *& bsdf, MemoryPool & customPool)
{
    void* buffer = customPool.allocate<GGXTransparentBSDF>();
    bsdf = new (buffer)GGXTransparentBSDF(DG, m_ior, m_ag, color, &customPool);
    bsdf->setEmission(this->getLight());
}

void GGXTransparentMaterial::setRoughness(float roughness)
{
    m_ag = roughness;
}
