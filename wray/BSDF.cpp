#include "StdAfx.h"
#include "BSDF.h"

WBSDF::~WBSDF(void)
{
}
void WBSDF::setDifferentialGeometry(WDifferentialGeometry &iDG)
{
	this->DG=iDG;
}

WVector3 WLambertBSDF::evaluateFCos(WVector3&ri, const WVector3&ro)
{
	//当颜色为纯白（1,1,1）时，对应的BSDF值为（1,1,1）/PI
//	WVector3 fCos=color*M_INV_PI*abs(ri.dot(DG.normal));
//	fCos.showCoords();
	if (ro.dot(DG.normal) <= 0.0f)
		return WVector3(0.0f);
	return color*M_INV_PI*max(ri.dot(DG.normal), 0.0f);
}
void WLambertBSDF::sampleRay(float u, float v, WVector3 &sampleWi, WVector3 &wo, float &pdf)
{
	WVector3 localVector;
	WMonteCarlo::cosineSampleHemisphere(u,v,localVector,pdf);
	sampleWi=//localVector;//DG.normal;
		DG.tangent*localVector.x+
		DG.bitangent*localVector.y+
		DG.normal*localVector.z;
//	sampleWi.normalize();
}
WVector3 WPhongBSDF::evaluateFCos(WVector3&ri,const WVector3&ro)
{
	float cosTheta=max(ri.reflect(DG.normal).dot(ro),0.0f);
	WVector3 colorWithSpecular=color+WVector3(pow(cosTheta,glossiness)*specular);
	return colorWithSpecular*M_INV_PI*max(ri.dot(DG.normal),0.0f);
}
void WPerfectReflectionBSDF::sampleRay(float u, float v, WVector3 &sampleWi, WVector3 &wo, float &pdf)
{
	sampleWi=wo.reflect(DG.normal);
	pdf=1.0f;
}
WVector3 WPerfectReflectionBSDF::evaluateFCos(WVector3&ri,const WVector3&ro)
{
	WVector3 riRef=ri.reflect(DG.normal);
	if((riRef-ro).length()<1e-3f)
		return color/**abs(ri.dot(DG.normal))*/;
	else return WVector3(0);
}

void WPerfectRefractionBSDF::sampleRay(float u, float v, WVector3 &sampleWi, WVector3 &wo, float &pdf)
{
	sampleWi=refractionDirection(wo,DG.normal);
	pdf=1.0f;
}
WVector3 WPerfectRefractionBSDF::refractionDirection(
	WVector3&i,const WVector3&normal)
{
	float cosThetaI=i.dot(normal);
	float cosThetaR;
	float ior;
	if(cosThetaI>=0)
	{
		ior=1.0f/IOR;
		cosThetaR=sqrt(1.0f-ior*ior*(1.0f-cosThetaI*cosThetaI));
		return -1.0f*ior*i-(cosThetaR-ior*cosThetaI)*normal;
	}
	else
	{
		cosThetaI*=-1.0f;
		ior=IOR;
		float cosThetaRSquared=1.0f-ior*ior*(1.0f-cosThetaI*cosThetaI);
		if(cosThetaRSquared<=0)
			return i.reflect(normal);
		cosThetaR=sqrt(cosThetaRSquared);
		return -1.0f*ior*i+(cosThetaR-ior*cosThetaI)*normal;
	}
}
WVector3 WPerfectRefractionBSDF::evaluateFCos(WVector3 &ri, const WVector3 &ro)
{
	WVector3 riRef=refractionDirection(ri,DG.normal);
	if((riRef-ro).length()<1e-3f)
		return color/**abs(ri.dot(DG.normal))*/;
	else return WVector3(0);
}
WFresnelDielectric::WFresnelDielectric(float iIOR,WVector3 inormal):
IOR(iIOR),normal(inormal)
{}

WVector3 WFresnelDielectric::refractionDirection(
	WVector3&i,const WVector3&nor)
{
	float cosThetaI=i.dot(nor);
	float cosThetaR;
	float ior;
	if(cosThetaI>=0)
	{
		ior=1.0f/IOR;
		cosThetaR=sqrt(1.0f-ior*ior*(1.0f-cosThetaI*cosThetaI));
		return -1.0f*ior*i-(cosThetaR-ior*cosThetaI)*nor;
	}
	else
	{
		cosThetaI*=-1.0f;
		ior=IOR;
		float cosThetaRSquared=1.0f-ior*ior*(1.0f-cosThetaI*cosThetaI);
		if(cosThetaRSquared<=0)
			return i.reflect(nor);
		cosThetaR=sqrt(cosThetaRSquared);
		return -1.0f*ior*i+(cosThetaR-ior*cosThetaI)*nor;
	}
}
float WFresnelDielectric::computeR(
		 float cosWi,float cosWt,float etaTO)
{
//	cout<<cosWi<<cosWt<<endl;
	float rParallel=(etaTO*cosWi-cosWt)/(etaTO*cosWi+cosWt);
	float rPerpendicular=(cosWi-etaTO*cosWt)/(cosWi+etaTO*cosWt);
//	cout<<rParallel<<' '<<rPerpendicular<<endl;
	return 0.5*(rParallel*rParallel+rPerpendicular*rPerpendicular);
}
float WFresnelDielectric::evaluateF(WVector3&wi)
{
	float cosWi=wi.dot(normal);
	float ior,cosWt;
	if(cosWi>=0)
	{
		ior=1/IOR;
		cosWt=sqrt(1.0f-ior*ior*(1.0f-cosWi*cosWi));
//		cout<<cosWt<<endl;
		return computeR(cosWi,cosWt,IOR);
	}
	else
	{
		cosWi*=-1.0f;
		ior=IOR;
		float cosWtSquared=1.0f-ior*ior*(1.0f-cosWi*cosWi);
		if(cosWtSquared<=0)
			return 1.0f;
		cosWt=sqrt(cosWtSquared);
		return computeR(cosWi,cosWt,1/ior);
	}
}
WFresnelConductor::WFresnelConductor(
			WVector3 ieta,WVector3 ik,WVector3 inormal):
eta(ieta),k(ik),normal(inormal)
{}
WVector3 WFresnelConductor::evaluateF(WVector3&wi)
{
	WVector3 cosWi=WVector3(max(0.01,wi.dot(normal)));
	WVector3 cosWi2=cosWi*cosWi;
	WVector3 n2k2=eta*eta+k*k;
// 	eta.showCoords();
// 	k.showCoords();
//	n2k2.showCoords();
	WVector3 rpa2=(n2k2*cosWi2-2*eta*cosWi+WVector3(1))/
				(n2k2*cosWi2+2*eta*cosWi+WVector3(1));
	WVector3 rpe2=(n2k2-2*eta*cosWi+cosWi2)/
				(n2k2+2*eta*cosWi+cosWi2);
//	rpa2.showCoords();
//	rpe2.showCoords();
	return 0.5*(rpa2+rpe2);
}
float WMetalBSDF::computeD(const WVector3&wh)
{
	float coswh=max(0.01f,DG.normal.dot(wh));
	return (exp+2.0f)*pow(coswh,exp)/(2*M_PI);
}
float WMetalBSDF::computeG(const WVector3&wi,const WVector3&wo,const WVector3&wh)
{
	float NH=DG.normal.dot(wh);
	float NWo=DG.normal.dot(wo);
	float NWi=DG.normal.dot(wi);
	WVector3 Wo=wo;
	float WoH=Wo.dot(wh);
	return max(0.01,min(1.0f,min(NWo,NWi)*2*NH/WoH));
}
WVector3 WMetalBSDF::evaluateFCos(WVector3&ri,const WVector3&ro)
{
	WVector3 rh=ri+ro;
	rh.normalize();
	fresnel.setNormal(rh);
	float cosWo=max(DG.normal.dot(ro),0.05);
	//     	cout<<computeD(rh)<<' '
	//     		<<computeG(ri,ro,rh)<<' ';
	//  	cout<<1/cosWo<<endl;
	//	fresnel.evaluateF(ri).showCoords();
	WVector3 Fcos=//WVector3(1,0.1,0.1)*//;
		computeD(rh)*
		computeG(ri,ro,rh)*
		fresnel.evaluateF(ri)
		/(4*cosWo)
		;
// 	if(Fcos.lengthSquared()>100)
//		return WVector3(1);
/*
	if(!(Fcos.length()<10))
	{
	cout<<Fcos.length();
	Fcos.showCoords();
	}*/

	return Fcos;
}
float WMetalBSDF::computePDF(WVector3&wi,WVector3&wo)
{
	WVector3 H=wi+wo;
	H.normalize();
	DG.normal.normalize();
	float cosH=max(0.05,H.dot(DG.normal));
	// 	cout<<cosH<<ends;
	// 	cout<<pow(cosH,exp)<<endl;
	//	cout<<wo.dot(H)<<endl;
	//	cout<<pow(cosH,exp)<<endl;
	float pdf=max((exp+1)*pow(cosH,exp)/(8*M_PI*wo.dot(H)),0.05);
	return pdf;
}
void WMetalBSDF::sampleRay(float u,float v, WVector3&sampleWi,WVector3&wo, float&pdf)
{
	float cosTheta=pow(u,1.0f/(exp+1.0f));
	float sinTheta=sqrt(max(0.0f,1.0f-cosTheta*cosTheta));
	float phi=v*2.0*M_PI;
	WVector3 localH(sinTheta*cos(phi),sinTheta*sin(phi),cosTheta);
	WVector3 H=
		localH.x*DG.tangent+
		localH.y*DG.bitangent+
		localH.z*DG.normal;
	sampleWi=wo.reflect(H);
	sampleWi.normalize();
	pdf=computePDF(sampleWi,wo);
}

float WDielectricBSDF::computeD(const WVector3&wh)
{
	float coswh=max(0.01f,DG.normal.dot(wh));
	return (exp+2.0f)*pow(coswh,exp)/(2*M_PI);
}
float WDielectricBSDF::computeG(const WVector3&wi,const WVector3&wo,const WVector3&wh)
{
	float NH=DG.normal.dot(wh);
	float NWo=DG.normal.dot(wo);
	float NWi=DG.normal.dot(wi);
	WVector3 Wo=wo;
	float WoH=Wo.dot(wh);
	return max(0.01,min(1.0f,min(NWo,NWi)*2*NH/WoH));
}
WVector3 WDielectricBSDF::evaluateFCos(WVector3&ri,const WVector3&ro)
{
	WVector3 rh=ri+ro;
	rh.normalize();
	fresnel.setNormal(rh);
	float cosWo=max(DG.normal.dot(ro),0.05);
	//     	cout<<computeD(rh)<<' '
	//     		<<computeG(ri,ro,rh)<<' ';
	//  	cout<<1/cosWo<<endl;
//		cout<<fresnel.evaluateF(ri)<<endl;
	float F=fresnel.evaluateF(ri);
//	cout<<F<<ends;
	WVector3 eColor=color*max(ri.dot(DG.normal),0.0f);
	WVector3 Fcos=
		computeD(rh)*
		computeG(ri,ro,rh)*
		WVector3(F)
		/(4*cosWo)+
		eColor*(1-F)
		;
	// 	if(Fcos.lengthSquared()>100)
	// 		return WVector3(1);
//	Fcos.showCoords();
	return Fcos;
}
float WDielectricBSDF::computePDF(WVector3&wi,WVector3&wo)
{
	WVector3 H=wi+wo;
	H.normalize();
	DG.normal.normalize();
	float cosH=H.dot(DG.normal);
	// 	cout<<cosH<<ends;
	// 	cout<<exp<<endl;
	//	cout<<wo.dot(H)<<endl;
	//	cout<<pow(cosH,exp)<<endl;
	float pdf= (exp+1)*pow(cosH,exp)/(8*M_PI*wo.dot(H));
	//	cout<<pdf<<endl;
	return pdf;
}
void WDielectricBSDF::sampleRay(float u,float v, WVector3&sampleWi,WVector3&wo, float&pdf)
{
	float cosTheta=pow(u,1.0f/(exp+1.0f));
	//	cout<<cosTheta<<endl;
	float sinTheta=sqrt(max(0.0f,1.0f-cosTheta*cosTheta));
	float phi=v*2.0*M_PI;
	WVector3 localH(sinTheta*cos(phi),sinTheta*sin(phi),cosTheta);
	WVector3 H=
		localH.x*DG.tangent+
		localH.y*DG.bitangent+
		localH.z*DG.normal;
	sampleWi=wo.reflect(H);
	sampleWi.normalize();
	pdf=computePDF(sampleWi,wo);
}

