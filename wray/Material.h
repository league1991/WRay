#pragma once
#include "BSDF.h"
class Material
{
public:
	enum MaterialType{
		MATERIAL_LAMBERT=0,
		MATERIAL_PHONG=1,
		MATERIAL_PERFECTREFLECTION=2,
		MATERIAL_PERFECTREFRACTION=3,
		MATERIAL_METAL=4,
		MATERIAL_DIELECTRIC=5,
		MATERIAL_GGX_METAL=6,
        MATERIAL_GGX_OPAQUE=7,
	};


	Material(
		MaterialType itype,string iName,unsigned int iID,
		Vector3 icolor=Vector3(1),Vector3 iLight=Vector3(0)):
	type(itype),name(iName),ID(iID),
	color(icolor),light(iLight){}

	virtual ~Material(void);
	//由材质创建BSDF,bsdf指针不需要预先新建对象
	//由此函数新建对象
	virtual void buildBSDF(
		DifferentialGeometry DG,BSDF*&bsdf, MemoryPool& customPool)=0;
	static void freeBSDF(BSDF* bsdf, MemoryPool& customPool)
	{
		customPool.free<BSDF>(bsdf);
	}

	virtual void setColor(Vector3 icolor){color=icolor;}
	Vector3 getColor(){return color;}
	Vector3 getLight(){return light;}
	virtual void getProperties(vector<float>& properties){};
	MaterialType getType(){return type;}
	bool     isEmissive(){return light.lengthSquared() > 1e-3f;}
protected:
	Vector3 light;
	Vector3 color;
	MaterialType type;
	string name;
	unsigned int ID;
};
class LambertMaterial:public Material
{
public:
	LambertMaterial(string iName,unsigned int iID,
		Vector3 icolor, Vector3 ilight=Vector3(0)):
	Material(MATERIAL_LAMBERT,iName,iID,icolor,ilight){}
	~LambertMaterial(){}

	//设置颜色
//	void setColor(Vector3 icolor){color=icolor;}
	//创建一个LambertBSDF
	void buildBSDF(DifferentialGeometry DG,BSDF*&bsdf, MemoryPool& customPool);
	void getProperties(vector<float>& properties);

};
class PhongMaterial:public LambertMaterial
{
public:
	PhongMaterial(string iName,unsigned int iID,
		const Vector3& icolor,const Vector3& ispecular,float iglossiness, Vector3& ilight=Vector3(0)):
	LambertMaterial(iName,iID,icolor,ilight),
		specular(ispecular),
		glossiness(iglossiness){type=MATERIAL_PHONG;}
	void setParams(float ispecular,
		float iglossiness)
	{specular=ispecular;
	glossiness=iglossiness;}
	void setSpecular(float ispecular)
	{specular=ispecular;}
	void setRoughness(float iglossiness)
	{glossiness=iglossiness;}
	void buildBSDF(DifferentialGeometry DG,BSDF*&bsdf, MemoryPool& customPool);
protected:
	Vector3 specular;
	float glossiness;
};
//完全光滑表面的材质
class PerfectReflectionMaterial:public Material
{
public:
	PerfectReflectionMaterial(string iName,unsigned int iID,
		Vector3 icolor=Vector3(1), Vector3 ilight=Vector3(0)):
	  Material(
		  MATERIAL_PERFECTREFLECTION,iName,iID,icolor,ilight)
	  {}
	  ~PerfectReflectionMaterial(){}


//	  void setColor(Vector3 icolor){color=icolor;}
	  void buildBSDF(DifferentialGeometry DG,BSDF*&bsdf, MemoryPool& customPool);
	  void getProperties(vector<float>& properties);
private:

};
//完全透明且光滑物体的材质
class PerfectRefractionMaterial:public Material
{
public:
	PerfectRefractionMaterial(string iName,unsigned int iID,
		Vector3 icolor=Vector3(1),float iIOR=1.33, Vector3 ilight=Vector3(0)):
	Material(
		  MATERIAL_PERFECTREFRACTION,iName,iID,
		  icolor,ilight),IOR(iIOR){}
	~PerfectRefractionMaterial(){}

	void buildBSDF(DifferentialGeometry DG,BSDF*&bsdf, MemoryPool& customPool);
	void setIOR(float ior){IOR=ior;}

	void getProperties(vector<float>& properties);
private:

	float IOR;
};
class MetalMaterial:public Material
{
public:
	MetalMaterial(string iName,unsigned int iID,Vector3 Fr,float iexp, Vector3 ilight=Vector3(0));
	~MetalMaterial(){}
	void buildBSDF(DifferentialGeometry DG,BSDF*&bsdf, MemoryPool& customPool);
	void refreshColor();
	void setRoughness(float iglossiness)
	{exp=iglossiness;}
private:
	Vector3 k,eta;
	float exp;
};

class GGXMetalMaterial :public Material
{
public:
	GGXMetalMaterial(string iName, unsigned int iID, Vector3 Fr, float iexp, Vector3 ilight = Vector3(0));
	~GGXMetalMaterial() {}
	void buildBSDF(DifferentialGeometry DG, BSDF*&bsdf, MemoryPool& customPool);
	void refreshColor();
	void setRoughness(float roughness);
private:
	Vector3 k, eta;
	float exp;
};

class GGXOpaqueMaterial : public Material
{
public:
    GGXOpaqueMaterial(string name, unsigned int ID,
        Vector3 color, float ag, float ior, Vector3 light = Vector3(0)) :
        Material(Material::MATERIAL_DIELECTRIC, name, ID, color, light),
        m_ag(ag), m_ior(ior) {}

    void buildBSDF(DifferentialGeometry DG, BSDF *& bsdf, MemoryPool & customPool);

    void setRoughness(float roughness);
private:
    float m_ag, m_ior;
};

class GGXTransparentMaterial : public Material
{
public:
    GGXTransparentMaterial(string name, unsigned int ID,
        Vector3 color, float ag, float ior, Vector3 light = Vector3(0)) :
        Material(Material::MATERIAL_DIELECTRIC, name, ID, color, light),
        m_ag(ag), m_ior(ior) {}

    void buildBSDF(DifferentialGeometry DG, BSDF *& bsdf, MemoryPool & customPool);

    void setRoughness(float roughness);
private:
    float m_ag, m_ior;
};

class DielectricMaterial:public Material
{
public:
	DielectricMaterial(string iName,unsigned int iID,
		Vector3 icolor,float iexp,float iior, Vector3 ilight=Vector3(0)):
	Material(Material::MATERIAL_DIELECTRIC,iName,iID,icolor,ilight),
	exp(iexp),ior(iior){}
	~DielectricMaterial(){}
	void buildBSDF(DifferentialGeometry DG,BSDF*&bsdf, MemoryPool& customPool);
	void setParams(float iglossiness,float iior)
	{exp=iglossiness;iior=ior;}
	void setRoughness(float iglossiness)
	{exp=iglossiness;}
	void setIOR(float iior)
	{ior=iior;}
private:
	float exp,ior;
};