#pragma once
#include "DifferentialGeometry.h"
#include "MC.h"

class BSDF
{
public:
	enum BSDFType {
		BSDF_LAMBERT = 0,
		BSDF_PHONG = 1,
		BSDF_PERFECTREFLECTION = 2,
		BSDF_PERFECTREFRACTION = 3,
		BSDF_METAL = 4,
		BSDF_DIELECTRIC = 5,
		BSDF_GGX_METAL = 6,
        BSDF_GGX_OPAQUE = 7,
	};

	BSDF(const DifferentialGeometry& iDG, BSDFType itype, MemoryPool* pool,
		bool iisEmissive = false, const Vector3& iemission = Vector3(0)) :
		DG(iDG), type(itype), isEmissive(iisEmissive), emission(0), m_memoryPool(pool) {};
	virtual ~BSDF(void);

	//对BSDF求值,顺带乘以入射角余弦
	//ri ro必须先单位化
	virtual Vector3 evaluateFCos(Vector3&ri, const Vector3&ro) = 0;
	//根据随机采样的u v值计算采样的光线
	//pdf为对应位置的概率密度
	//sampleWi是要计算的采样光线
	virtual void sampleRay(float u, float v, Vector3&sampleWi, const Vector3&wo, float&pdf) = 0;
	virtual Vector3 rho(Vector3&wo) { return Vector3(0); }
	virtual Vector3 getEmission() { return emission; }
	virtual void     setEmission(const Vector3& emi) { emission = emi; }
	virtual bool isDeltaBSDF() = 0;
	//各种get函数
	BSDFType getType() { return type; }
	Vector3 getPosition() { return DG.position; }
	Vector3 getNormal() { return DG.normal; }
	Vector3 getBitangent() { return DG.bitangent; }
	Vector3 getTangent() { return DG.tangent; }
	Vector2f getTexCoord() { return DG.texCoord; }

	//各种set函数
	//设置DG
	void setDifferentialGeometry(
		DifferentialGeometry&iDG);
	unsigned int getMtlID() { return DG.mtlId; }
	BSDFType type;
	DifferentialGeometry DG;
	bool isEmissive;
protected:
	Vector3 emission;
    MemoryPool* m_memoryPool;
};

class LambertBSDF :public BSDF
{
public:
	LambertBSDF(const DifferentialGeometry& iDG, const Vector3 &icolor, MemoryPool* pool) :
		BSDF(iDG, BSDF_LAMBERT, pool), color(icolor) {}
	~LambertBSDF() {}

	//随机选择一条光线方向
	//光线的方向服从余弦分布
	void sampleRay(
		float u, float v,
		Vector3&sampleWi, const Vector3&wo, float&pdf);
	//此为BSDF求值函数
	//返回颜色值/PI
	//因为根据能量守恒，应有Integrate[F*cos(thetaI)*dwi]<=1
	//证明：首先BSDF是表示当有一束光从一个方向入射时，
	//其所产生的反射光的分布情况，
	//或者说，考虑有几束方向相近（在dwi立体角内）的入射光线
	//在一个特定方向，BSDF就是
	//那几束入射光线在这个方向上贡献的能量占其总能量的比值
	//F=dLo/dEi=dLo/(Li*cos(thetaI)*dwi)
	//代入，积分式子化为Integrate[dLo/Li]
	//当一个方向入射的光线被不吸收地均匀反射到各个方向时，
	//上述积分为1
	//又因为对于Lambert材质，入射光线被均匀地散射到各个方向，
	//故F为一个常数，当积分为1,
	//即入射的光线被不吸收地均匀反射到各个方向，即为纯白色时
	//F为1/PI
	//当存在吸收，即颜色不为纯白时，只需在此基础上乘以
	//颜色的规格化向量（白色为（1,1,1））
	Vector3 evaluateFCos(Vector3&ri, const Vector3&ro);
	Vector3 rho(Vector3&wo) { return color; }
	virtual bool isDeltaBSDF() { return false; }
protected:
	Vector3 color;
};

class PhongBSDF :public LambertBSDF
{
public:
	PhongBSDF(const DifferentialGeometry& iDG, const Vector3& icolor,
		const Vector3& ispecular, float iglossiness, MemoryPool* pool) :
		LambertBSDF(iDG, icolor, pool), specular(ispecular), glossiness(iglossiness)
	{
		type = BSDF_PHONG;
	}
	Vector3 evaluateFCos(Vector3&ri, const Vector3&ro);
	virtual bool isDeltaBSDF() { return false; }
private:
	Vector3 specular;
	float glossiness;
};


//这个BSDF模拟完全光滑表面的反射
class PerfectReflectionBSDF :public BSDF
{
public:
	PerfectReflectionBSDF(const DifferentialGeometry& iDG,
		Vector3& icolor, MemoryPool* pool) :BSDF(iDG, BSDF::BSDF_PERFECTREFLECTION, pool), color(icolor) {}
	~PerfectReflectionBSDF() {};
	void sampleRay(
		float u, float v,
		Vector3&sampleWi, const Vector3&wo,
		float&pdf);

	//注意，由于为完全光滑表面，反射光线方向严格遵循反射定律
	//又由于上述BSDF的定义，BSDF为
	//方向相近的几束入射光线
	//在一个特定出射方向上贡献的能量占其总能量的比值
	//故对于完全光滑表面，bsdf不为一个常数，而是一个delta函数
	//所以不能用MC方法估计，但可以通过直接计算积分来求得反射光
	Vector3 evaluateFCos(Vector3&ri, const Vector3&ro);
	virtual bool isDeltaBSDF() { return true; }
private:
	Vector3 color;
};
//这个BSDF模拟完全光滑表面的折射
class PerfectRefractionBSDF :public BSDF
{
public:
	PerfectRefractionBSDF(const DifferentialGeometry& iDG,
		Vector3& icolor, float iIOR, MemoryPool* pool) :BSDF(iDG, BSDF::BSDF_PERFECTREFRACTION, pool), color(icolor), IOR(iIOR) {}
	~PerfectRefractionBSDF() {}
	void sampleRay(
		float u, float v,
		Vector3&sampleWi, const Vector3&wo,
		float&pdf);

	Vector3 evaluateFCos(Vector3&ri, const Vector3&ro);
	virtual bool isDeltaBSDF() { return true; }
private:
	Vector3 color;
	float IOR;

	//计算折射光线方向，已包含全反射效果
	Vector3 refractionDirection(
		const Vector3&i, const Vector3&normal);
};

//辅助类，用于计算菲涅尔系数
//这个类针对绝缘体
class FresnelDielectric
{
public:
	FresnelDielectric(float iIOR, Vector3 inormal);

	//计算折射光线方向，已包含全反射效果
	Vector3 refractionDirection(
		const Vector3&i, const Vector3&normal);
	float evaluateF(const Vector3&wi);
	void setNormal(const Vector3&inormal) { normal = inormal; }
	virtual bool isDeltaBSDF() { return false; }
    float getIOR() const { return IOR; }
private:
	//计算r值 ，etaTO为折射率
	float computeR(float cosWi, float cosWt, float etaTO);
	//IOR为折射率，IOR=物体折射率/空气折射率
	float IOR;
	Vector3 normal;
};
//这个类针对导体
class FresnelConductor
{
public:
	FresnelConductor(Vector3 ieta, Vector3 ik, Vector3 inormal);
	Vector3 evaluateF(const Vector3&wi);
	void setNormal(const Vector3&inormal) { normal = inormal; }
private:
	Vector3 eta, k, normal;

};

//模拟金属材质
class MetalBSDF :public BSDF
{
public:
	MetalBSDF(const DifferentialGeometry& iDG,
		Vector3 ieta, Vector3 ik, float iexp, MemoryPool* pool) :
		BSDF(iDG, BSDF_METAL, pool),
		fresnel(ieta, ik, iDG.normal),
		exp(iexp) {}
	~MetalBSDF() {}
	void sampleRay(float u, float v, Vector3&sampleWi, const Vector3&wo, float&pdf);
	Vector3 evaluateFCos(Vector3&ri, const Vector3&ro);
	virtual bool isDeltaBSDF() { return false; }
private:
	float computeD(const Vector3&wh);
	float computeG(const Vector3&wi, const Vector3&wo, const Vector3&wh);
	float computePDF(const Vector3&wi, const Vector3&wo);
	FresnelConductor fresnel;
	float exp;
};

class GGXDistribution
{
public:
    GGXDistribution(DifferentialGeometry* dg, float ag) :m_ag(ag), m_dg(dg) {}
    
    float computeD(const Vector3 & wh) const;
    float computeG1(const Vector3 & v, const Vector3 & wh) const;
    float computeG(const Vector3 & wi, const Vector3 & wo, const Vector3 & wh) const
    {
        return computeG1(wi, wh) * computeG1(wo, wh);
    }
    float computePDF(const Vector3 & wi, const Vector3 & wo);

    float ag() const { return m_ag; }
    Vector3 sampleRay(float u, float v, Vector3&sampleWi, const Vector3&wo) const;
private:
    float m_ag;
    DifferentialGeometry* m_dg;
};

class GGXMetalBSDF :public BSDF
{
public:
	GGXMetalBSDF(const DifferentialGeometry& iDG, Vector3 ieta, Vector3 ik, float ag, MemoryPool* pool) :
		BSDF(iDG, BSDF_GGX_METAL, pool), fresnel(ieta, ik, iDG.normal), m_distribution(&DG, ag) {}
	~GGXMetalBSDF() {}
	void sampleRay(float u, float v, Vector3&sampleWi, const Vector3&wo, float&pdf);
	Vector3 evaluateFCos(Vector3&ri, const Vector3&ro);
	virtual bool isDeltaBSDF() { return false; }
private:
    GGXDistribution  m_distribution;
	FresnelConductor fresnel;
};

class GGXOpaqueBSDF : public BSDF
{
public:
    GGXOpaqueBSDF(const DifferentialGeometry& iDG, float ior,
        Vector3 icolor, float ag, MemoryPool* pool) :
        BSDF(iDG, BSDF_GGX_OPAQUE, pool),
        m_fresnel(ior, iDG.normal),
        m_distribution(&DG, ag),
        m_diffuseColor(icolor) {}

    void sampleRay(float u, float v, Vector3&sampleWi, const Vector3&wo, float&pdf);

    Vector3 evaluateFCos(Vector3&ri, const Vector3&ro);
    virtual bool isDeltaBSDF() { return false; }
private:
    GGXDistribution  m_distribution;
    FresnelDielectric m_fresnel;
    Vector3 m_diffuseColor;
};

class GGXTransparentBSDF : public BSDF
{
public:
    GGXTransparentBSDF(const DifferentialGeometry& iDG, float ior,float ag, const Vector3& color, MemoryPool* pool) :
        BSDF(iDG, BSDF_GGX_OPAQUE, pool),
        m_fresnel(ior, iDG.normal),
        m_distribution(&DG, ag),
        m_color(color) {}

    void sampleRay(float u, float v, Vector3&sampleWi, const Vector3&wo, float&pdf);

    Vector3 evaluateFCos(Vector3&ri, const Vector3&ro);

    virtual bool isDeltaBSDF() { return false; }
private:
    GGXDistribution  m_distribution;
    FresnelDielectric m_fresnel;
    Vector3 m_color;
};

//此BSDF表示不透明绝缘体的BSDF
class DielectricBSDF :public BSDF
{
public:
	DielectricBSDF(const DifferentialGeometry& iDG, float iior,
		Vector3 icolor, float iexp, MemoryPool* pool) :
		BSDF(iDG, BSDF_DIELECTRIC, pool),
		fresnel(iior, iDG.normal),
		color(icolor), exp(iexp) {}
	~DielectricBSDF() {}
	void sampleRay(
		float u, float v,
		Vector3&sampleWi, const Vector3&wo,
		float&pdf);

	Vector3 evaluateFCos(Vector3&ri, const Vector3&ro);
	virtual bool isDeltaBSDF() { return false; }
private:
	float computeD(const Vector3&wh);
	float computeG(const Vector3&wi, const Vector3&wo, const Vector3&wh);
	float computePDF(const Vector3&wi, const Vector3&wo);
	FresnelDielectric fresnel;
	float IOR;
	float exp;
	Vector3 color;
};
