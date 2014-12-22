#pragma once
#include "DifferentialGeometry.h"
#include "MC.h"

class WBSDF
{
public:
	enum BSDFType{
		BSDF_LAMBERT=0,
		BSDF_PHONG=1,
		BSDF_PERFECTREFLECTION=2,
		BSDF_PERFECTREFRACTION=3,
		BSDF_METAL=4,
		BSDF_DIELECTRIC=5
	};

	WBSDF(WDifferentialGeometry& iDG,BSDFType itype,
		bool iisEmissive=false, const WVector3& iemission=WVector3(0)):
	DG(iDG),type(itype),isEmissive(iisEmissive),emission(0){};
	virtual ~WBSDF(void);

	//对BSDF求值,顺带乘以入射角余弦
	//ri ro必须先单位化
	virtual WVector3 evaluateFCos(WVector3&ri,const WVector3&ro)=0;
	//根据随机采样的u v值计算采样的光线
	//pdf为对应位置的概率密度
	//sampleWi是要计算的采样光线
	virtual void sampleRay(
		float u,float v,WVector3&sampleWi,WVector3&wo,float&pdf)=0;
	virtual WVector3 rho(WVector3&wo){return WVector3(0);}
	virtual WVector3 getEmission(){return emission;}
	virtual void     setEmission(const WVector3& emi){emission = emi;}

	//各种get函数
	BSDFType getType(){return type;}
	WVector3 getPosition(){return DG.position;}
	WVector3 getNormal(){return DG.normal;}
	WVector3 getBitangent(){return DG.bitangent;}
	WVector3 getTangent(){return DG.tangent;}
	WVector2 getTexCoord(){return DG.texCoord;}

	//各种set函数
	//设置DG
	void setDifferentialGeometry(
		WDifferentialGeometry&iDG);
	unsigned int getMtlID(){return DG.mtlId;}	
	BSDFType type;
	WDifferentialGeometry DG;
	bool isEmissive;
private:
	WVector3 emission;

};

class WLambertBSDF:public WBSDF
{
public:
	WLambertBSDF(WDifferentialGeometry& iDG,WVector3 &icolor):
	  WBSDF(iDG,BSDF_LAMBERT),color(icolor){}
	~WLambertBSDF(){}

	//随机选择一条光线方向
	//光线的方向服从余弦分布
	void sampleRay(
		float u,float v,
		WVector3&sampleWi,WVector3&wo,float&pdf);
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
	WVector3 evaluateFCos(WVector3&ri,const WVector3&ro);
	WVector3 rho(WVector3&wo){return color;}
protected:
	WVector3 color;
};

class WPhongBSDF:public WLambertBSDF
{
public:
	WPhongBSDF(WDifferentialGeometry& iDG,WVector3& icolor,
		float ispecular,float iglossiness):
	WLambertBSDF(iDG,icolor),specular(ispecular),glossiness(iglossiness)
	{type=BSDF_PHONG;}
	WVector3 evaluateFCos(WVector3&ri,const WVector3&ro);
private:
	float specular,glossiness;
};


//这个BSDF模拟完全光滑表面的反射
class WPerfectReflectionBSDF:public WBSDF
{
public:
	WPerfectReflectionBSDF(WDifferentialGeometry& iDG,
		WVector3& icolor):WBSDF(iDG,WBSDF::BSDF_PERFECTREFLECTION),color(icolor){}
	~WPerfectReflectionBSDF(){};
	void sampleRay(
		float u,float v,
		WVector3&sampleWi,WVector3&wo,
		float&pdf);

	//注意，由于为完全光滑表面，反射光线方向严格遵循反射定律
	//又由于上述BSDF的定义，BSDF为
	//方向相近的几束入射光线
	//在一个特定出射方向上贡献的能量占其总能量的比值
	//故对于完全光滑表面，bsdf不为一个常数，而是一个delta函数
	//所以不能用MC方法估计，但可以通过直接计算积分来求得反射光
	WVector3 evaluateFCos(WVector3&ri,const WVector3&ro);
private:
	WVector3 color;
};
//这个BSDF模拟完全光滑表面的折射
class WPerfectRefractionBSDF:public WBSDF
{
public:
	WPerfectRefractionBSDF(WDifferentialGeometry& iDG,
		WVector3& icolor,float iIOR):WBSDF(iDG,WBSDF::BSDF_PERFECTREFRACTION),color(icolor),IOR(iIOR){}
	~WPerfectRefractionBSDF(){}
	void sampleRay(
		float u,float v,
		WVector3&sampleWi,WVector3&wo,
		float&pdf);

	WVector3 evaluateFCos(WVector3&ri,const WVector3&ro);
private:
	WVector3 color;
	float IOR;

	//计算折射光线方向，已包含全反射效果
	WVector3 refractionDirection(
		WVector3&i,const WVector3&normal);
};

//辅助类，用于计算菲涅尔系数
//这个类针对绝缘体
class WFresnelDielectric
{
public:
	WFresnelDielectric(float iIOR,WVector3 inormal);

	//计算折射光线方向，已包含全反射效果
	WVector3 refractionDirection(
		WVector3&i,const WVector3&normal);
	float evaluateF(WVector3&wi);
	void setNormal(WVector3&inormal){normal=inormal;}
private:
	//计算r值 ，etaTO为折射率
	float computeR(float cosWi,float cosWt,float etaTO);
	//IOR为折射率，IOR=物体折射率/空气折射率
	float IOR;
	WVector3 normal;
};
//这个类针对导体
class WFresnelConductor
{
public:
	WFresnelConductor(WVector3 ieta,WVector3 ik,WVector3 inormal);
	WVector3 evaluateF(WVector3&wi);
	void setNormal(WVector3&inormal){normal=inormal;}
private:
	WVector3 eta,k,normal;

};

//模拟金属材质
class WMetalBSDF:public WBSDF
{
public:
	WMetalBSDF(WDifferentialGeometry iDG,
		WVector3 ieta,WVector3 ik,float iexp):
	  WBSDF(iDG,BSDF_METAL),
		  fresnel(ieta,ik,iDG.normal),
		  exp(iexp){}
	  ~WMetalBSDF(){}
	  void sampleRay(
		  float u,float v,
		  WVector3&sampleWi,WVector3&wo,
		  float&pdf);

	  WVector3 evaluateFCos(WVector3&ri,const WVector3&ro);
private:
	float computeD(const WVector3&wh);
	float computeG(const WVector3&wi,const WVector3&wo,const WVector3&wh);
	float computePDF(WVector3&wi,WVector3&wo);
	WFresnelConductor fresnel;
	float exp;
};

//此BSDF表示不透明绝缘体的BSDF
class WDielectricBSDF:public WBSDF
{
public:
	WDielectricBSDF(WDifferentialGeometry iDG,float iior,
		WVector3 icolor,float iexp):
	WBSDF(iDG,BSDF_DIELECTRIC),
		fresnel(iior,iDG.normal),
		color(icolor),exp(iexp){}
	~WDielectricBSDF(){}
	void sampleRay(
		float u,float v,
		WVector3&sampleWi,WVector3&wo,
		float&pdf);

	WVector3 evaluateFCos(WVector3&ri,const WVector3&ro);
private:
	float computeD(const WVector3&wh);
	float computeG(const WVector3&wi,const WVector3&wo,const WVector3&wh);
	float computePDF(WVector3&wi,WVector3&wo);
	WFresnelDielectric fresnel;
	float IOR;
	float exp;
	WVector3 color;
};
