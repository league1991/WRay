#include "StdAfx.h"
#include "BSDF.h"

BSDF::~BSDF(void)
{
}
void BSDF::setDifferentialGeometry(DifferentialGeometry &iDG)
{
	this->DG = iDG;
}

Vector3 LambertBSDF::evaluateFCos(Vector3&ri, const Vector3&ro)
{
	//当颜色为纯白（1,1,1）时，对应的BSDF值为（1,1,1）/PI
	if (ro.dot(DG.normal) <= 0.0f)
		return Vector3(0.0f);
	return color*M_INV_PI*max(ri.dot(DG.normal), 0.0f);
}
Vector3 LambertBSDF::sampleRay(float u, float v, Vector3 &sampleWi, const Vector3 &wo, float &pdf)
{
	Vector3 localVector;
	RandomNumber::cosineSampleHemisphere(u, v, localVector, pdf);
	sampleWi =
		DG.tangent*localVector.x +
		DG.bitangent*localVector.y +
		DG.normal*localVector.z;
    if (sampleWi.dot(DG.geometricNormal) < 0)
    {
        sampleWi *= -1;
    }
    return evaluateFCos(sampleWi, wo);
}
Vector3 PhongBSDF::evaluateFCos(Vector3&ri, const Vector3&ro)
{
	Vector3 H = 0.5 * (ri + ro);
	H.normalize();
	float cosTheta = max(DG.normal.dot(H), 0.0f);
	return color * (pow(cosTheta, glossiness) * (glossiness+2)/(2 * M_PI))*max(ri.dot(DG.normal), 0.0f);
}
Vector3 PerfectReflectionBSDF::sampleRay(float u, float v, Vector3 &sampleWi, const Vector3 &wo, float &pdf)
{
	sampleWi = wo.reflect(DG.normal);
	pdf = 1.0f;
    return evaluateFCos(sampleWi, wo);
}
Vector3 PerfectReflectionBSDF::evaluateFCos(Vector3&ri, const Vector3&ro)
{
	Vector3 riRef = ri.reflect(DG.normal);
	if ((riRef - ro).length() < 1e-3f)
		return color/**abs(ri.dot(DG.normal))*/;
	else return Vector3(0);
}

Vector3 PerfectRefractionBSDF::sampleRay(float u, float v, Vector3 &sampleWi, const Vector3 &wo, float &pdf)
{
	sampleWi = refractionDirection(wo, DG.normal);
	pdf = 1.0f;
    return evaluateFCos(sampleWi, wo);
}
Vector3 PerfectRefractionBSDF::refractionDirection(
	const Vector3&i, const Vector3&normal)
{
	float cosThetaI = i.dot(normal);
	float cosThetaR;
	float ior;
	if (cosThetaI >= 0)
	{
		ior = 1.0f / IOR;
		cosThetaR = sqrt(1.0f - ior*ior*(1.0f - cosThetaI*cosThetaI));
		return -1.0f*ior*i - (cosThetaR - ior*cosThetaI)*normal;
	}
	else
	{
		cosThetaI *= -1.0f;
		ior = IOR;
		float cosThetaRSquared = 1.0f - ior*ior*(1.0f - cosThetaI*cosThetaI);
		if (cosThetaRSquared <= 0)
			return i.reflect(normal);
		cosThetaR = sqrt(cosThetaRSquared);
		return -1.0f*ior*i + (cosThetaR - ior*cosThetaI)*normal;
	}
}
Vector3 PerfectRefractionBSDF::evaluateFCos(Vector3 &ri, const Vector3 &ro)
{
	Vector3 riRef = refractionDirection(ri, DG.normal);
	if ((riRef - ro).length() < 1e-3f)
		return color/**abs(ri.dot(DG.normal))*/;
	else return Vector3(0);
}
FresnelDielectric::FresnelDielectric(float iIOR, Vector3 inormal) :
	IOR(iIOR), normal(inormal)
{}

Vector3 FresnelDielectric::refractionDirection(
	const Vector3&i, const Vector3&nor)
{
	float cosThetaI = i.dot(nor);
	float cosThetaR;
	float ior;
	if (cosThetaI >= 0)
	{
		ior = 1.0f / IOR;
		cosThetaR = sqrt(1.0f - ior*ior*(1.0f - cosThetaI*cosThetaI));
		return -1.0f*ior*i - (cosThetaR - ior*cosThetaI)*nor;
	}
	else
	{
		cosThetaI *= -1.0f;
		ior = IOR;
		float cosThetaRSquared = 1.0f - ior*ior*(1.0f - cosThetaI*cosThetaI);
		if (cosThetaRSquared <= 0)
			return i.reflect(nor);
		cosThetaR = sqrt(cosThetaRSquared);
		return -1.0f*ior*i + (cosThetaR - ior*cosThetaI)*nor;
	}
}
float FresnelDielectric::computeR(
	float cosWi, float cosWt, float etaTO)
{
	float rParallel = (etaTO*cosWi - cosWt) / (etaTO*cosWi + cosWt);
	float rPerpendicular = (cosWi - etaTO*cosWt) / (cosWi + etaTO*cosWt);
	return 0.5f*(rParallel*rParallel + rPerpendicular*rPerpendicular);
}
float FresnelDielectric::evaluateF(const Vector3&wi)
{
	float cosWi = wi.dot(normal);
    float iorI = cosWi >= 0?1:IOR;
    float iorT = cosWi >= 0?IOR:1;
    float sinWi2 = 1 - cosWi * cosWi;
    float sinWt2 = sinWi2 * iorI * iorI / (iorT * iorT);
    if (sinWt2 > 1)
    {
        // Total internal reflection
        return 1.0;
    }
    float cosWt = sqrt(1 - sinWt2);
    return computeR(abs(cosWi), cosWt, iorT / iorI);
}
FresnelConductor::FresnelConductor(
	Vector3 ieta, Vector3 ik, Vector3 inormal) :
	eta(ieta), k(ik), normal(inormal)
{}
Vector3 FresnelConductor::evaluateF(const Vector3&wi)
{
	float cosWi = wi.dot(normal);
	if (cosWi <= 0.f)
	{
		return Vector3(0);
	}
	Vector3 cosWi2 = Vector3(cosWi* cosWi);
	Vector3 n2k2 = eta*eta + k*k;
	Vector3 rpa2A = (n2k2*cosWi2 - 2 * eta*cosWi + Vector3(1));
	Vector3 rpa2B = (n2k2*cosWi2 + 2 * eta*cosWi + Vector3(1));
	Vector3 rpe2A = (n2k2 - 2 * eta*cosWi + cosWi2);
	Vector3 rpe2B = (n2k2 + 2 * eta*cosWi + cosWi2);
	if (rpa2B.x <= 0 || rpa2B.y <= 0 || rpa2B.z <= 0 ||
		rpe2B.x <= 0 || rpe2B.y <= 0 || rpe2B.z <= 0 ||
		rpa2A.x <= 0 || rpa2A.y <= 0 || rpa2A.z <= 0 ||
		rpe2A.x <= 0 || rpe2A.y <= 0 || rpe2A.z <= 0)
	{
		return Vector3(0);
	}
	return 0.5*(rpa2A / rpa2B + rpe2A / rpe2B);
}
float MetalBSDF::computeD(const Vector3&wh)
{
	float coswh = max(0.01f, DG.normal.dot(wh));
	return (exp + 2.0f)*pow(coswh, exp) / (2 * float(M_PI));
}
float MetalBSDF::computeG(const Vector3&wi, const Vector3&wo, const Vector3&wh)
{
	float NH = DG.normal.dot(wh);
	float NWo = DG.normal.dot(wo);
	float NWi = DG.normal.dot(wi);
	Vector3 Wo = wo;
	float WoH = Wo.dot(wh);
	return max(0.01f, min(1.0f, min(NWo, NWi) * 2 * NH / WoH));
}
Vector3 MetalBSDF::evaluateFCos(Vector3&ri, const Vector3&ro)
{
	Vector3 rh = ri + ro;
	rh.normalize();
	fresnel.setNormal(rh);
	float cosWo = max(DG.normal.dot(ro), 0.05f);
	Vector3 Fcos = computeD(rh)*computeG(ri, ro, rh)* fresnel.evaluateF(ri) / (4 * cosWo);
	return Fcos;
}

float MetalBSDF::computePDF(const Vector3&wi, const Vector3&wo)
{
	Vector3 H = wi + wo;
	H.normalize();
	DG.normal.normalize();
	float cosH = max(0.05f, H.dot(DG.normal));
	float pdf = max((exp + 1)*pow(cosH, exp) / (8 * float(M_PI)*wo.dot(H)), 0.05f);
	return pdf;
}
Vector3 MetalBSDF::sampleRay(float u, float v, Vector3&sampleWi, const Vector3&wo, float&pdf)
{
	float cosTheta = pow(u, 1.0f / (exp + 1.0f));
	float sinTheta = sqrt(max(0.0f, 1.0f - cosTheta*cosTheta));
	float phi = v*2.0*M_PI;
	Vector3 localH(sinTheta*cos(phi), sinTheta*sin(phi), cosTheta);
	Vector3 H =
		localH.x*DG.tangent +
		localH.y*DG.bitangent +
		localH.z*DG.normal;
	sampleWi = wo.reflect(H);
	sampleWi.normalize();
	pdf = computePDF(sampleWi, wo);
    return evaluateFCos(sampleWi, wo);
}

float DielectricBSDF::computeD(const Vector3&wh)
{
	float coswh = max(0.01f, DG.normal.dot(wh));
	return (exp + 2.0f)*pow(coswh, exp) / (2 * M_PI);
}

float DielectricBSDF::computeG(const Vector3&wi, const Vector3&wo, const Vector3&wh)
{
	float NH = DG.normal.dot(wh);
	float NWo = DG.normal.dot(wo);
	float NWi = DG.normal.dot(wi);
	Vector3 Wo = wo;
	float WoH = Wo.dot(wh);
	return max(0.01f, min(1.0f, min(NWo, NWi) * 2 * NH / WoH));
}
Vector3 DielectricBSDF::evaluateFCos(Vector3&ri, const Vector3&ro)
{
	Vector3 rh = ri + ro;
	rh.normalize();
	fresnel.setNormal(rh);
	float cosWo = max(DG.normal.dot(ro), 0.05f);
	float F = fresnel.evaluateF(ri);
	Vector3 eColor = color*max(ri.dot(DG.normal), 0.0f);
	Vector3 Fcos =
		computeD(rh)*
		computeG(ri, ro, rh)*
		Vector3(F)
		/ (4 * cosWo) +
		eColor*(1 - F)
		;
	return Fcos;
}
float DielectricBSDF::computePDF(const Vector3&wi, const Vector3&wo)
{
	Vector3 H = wi + wo;
	H.normalize();
	DG.normal.normalize();
	float cosH = H.dot(DG.normal);
	float pdf = (exp + 1)*pow(cosH, exp) / (8 * M_PI*wo.dot(H));
	return pdf;
}
Vector3 DielectricBSDF::sampleRay(float u, float v, Vector3&sampleWi, const Vector3&wo, float&pdf)
{
	float cosTheta = pow(u, 1.0f / (exp + 1.0f));
	float sinTheta = sqrt(max(0.0f, 1.0f - cosTheta*cosTheta));
	float phi = v*2.0*M_PI;
	Vector3 localH(sinTheta*cos(phi), sinTheta*sin(phi), cosTheta);
	Vector3 H =
		localH.x*DG.tangent +
		localH.y*DG.bitangent +
		localH.z*DG.normal;
	sampleWi = wo.reflect(H);
	sampleWi.normalize();
	pdf = computePDF(sampleWi, wo);
    return evaluateFCos(sampleWi, wo);
}

Vector3 GGXMetalBSDF::sampleRay(float u, float v, Vector3 & sampleWi, const Vector3 & wo, float & pdf)
{
    float dummy;
    float ag = m_ag;
    GGXDistribution distribution(&DG, ag);
    Vector3 localH = distribution.sampleNormal(u, v);
	Vector3 H =
		localH.x*DG.tangent +
		localH.y*DG.bitangent +
		localH.z*DG.normal;
	sampleWi = wo.reflect(H);
	sampleWi.normalize();
	pdf = distribution.computePDF(sampleWi, wo);
    return evaluateFCos(sampleWi, wo);
}

Vector3 GGXMetalBSDF::evaluateFCos(Vector3 & ri, const Vector3 & ro)
{
	Vector3 rh = ri + ro;
	rh.normalize();
	fresnel.setNormal(rh);
	float cosWo = DG.normal.dot(ro);
	if (cosWo <= 0)
	{
		return Vector3(0);
	}
    GGXDistribution distribution(&DG, m_ag);
	Vector3 Fcos = distribution.computeD(rh)*distribution.computeG(ri, ro, rh) *fresnel.evaluateF(ri) / (4 * cosWo);
	return Fcos;
}

float GGXDistribution::computeD(const Vector3 & wh) const
{
    // Both sides are OK
    float cosThetaM = abs(wh.dot(m_dg->normal));
    float cosThetaM2 = cosThetaM * cosThetaM;
    float tanThetaM2 = 1 / (cosThetaM2)-1;
    float ag2 = m_ag * m_ag;
    float ag2tan2 = ag2 + tanThetaM2;
    return ag2 / (M_PI * cosThetaM2 * cosThetaM2 *ag2tan2*ag2tan2);
}

float GGXDistribution::computeG1(const Vector3 & v, const Vector3 & wh) const
{
    float vm = v.dot(wh);
    float vn = v.dot(m_dg->normal);
    if (vm * vn <= 0)
    {
        return 0.f;
    }
    float tanv2 = 1 / (vn*vn) - 1;
    float g1 = 2.f / (1.f + sqrt(1.f + m_ag*m_ag*tanv2));
    return g1;
}

float GGXDistribution::computePDF(const Vector3 & wi, const Vector3 & wo)
{
    Vector3 H = wi + wo;
    H.normalize();
    float pm = computeD(H) * abs(H.dot(m_dg->normal));
    float jacobian = 1.f / (4.f * wo.dot(H));
    return max(pm * jacobian, 0.0001f);
}

Vector3 GGXDistribution::sampleNormal(float u, float v, float* pdf) const
{
    float theta = atan(m_ag * sqrt(u) / sqrt(1.f - u));
    float phi = 2 * M_PI * v;
    float sinTheta = sin(theta);
    float cosTheta = cos(theta);
    Vector3 res(sinTheta*cos(phi), sinTheta*sin(phi), cosTheta);
    if (pdf)
    {
        float cosTheta2 = cosTheta * cosTheta;
        float tanTheta = sinTheta / cosTheta;
        float ag2 = m_ag * m_ag;
        float ag2tan = ag2 + tanTheta * tanTheta;
        *pdf = ag2 / (M_PI * cosTheta2 * cosTheta2 * ag2tan * ag2tan) * cosTheta;
    }
    return res;
}

Vector3 GGXOpaqueBSDF::sampleRay(float u, float v, Vector3 & sampleWi, const Vector3 & wo, float & pdf)
{
    Vector3 localVector;
    RandomNumber::cosineSampleHemisphere(u, v, localVector, pdf);
    sampleWi =
        DG.tangent*localVector.x +
        DG.bitangent*localVector.y +
        DG.normal*localVector.z;

    if (sampleWi.dot(DG.geometricNormal) < 0)
    {
        sampleWi *= -1;
    }
    return evaluateFCos(sampleWi, wo);
}

Vector3 GGXOpaqueBSDF::evaluateFCos(Vector3 & ri, const Vector3 & ro)
{
    Vector3 rh = ri + ro;
    rh.normalize();
    m_fresnel.setNormal(rh);
    float cosWo = DG.normal.dot(ro);
    if (cosWo <= 0)
    {
        return Vector3(0);
    }
    GGXDistribution distribution(&DG, m_ag);
    float D = distribution.computeD(rh);
    float G = distribution.computeG(ri, ro, rh);
    float F = m_fresnel.evaluateF(ri);
    float specular = D * G * F / (4 * cosWo);
    float diffuseFactor = M_INV_PI*max(ri.dot(DG.normal), 0.0f);
    Vector3 Fcos = Vector3(specular) + (1 - F)*diffuseFactor* m_diffuseColor;
    return Fcos;
}

Vector3 GGXTransparentBSDF::sampleRay(float u, float v, Vector3 & sampleWi, const Vector3 & wo, float & pdf)
{
    float pdfM;
    GGXDistribution distribution(&DG, m_ag);
    Vector3 localH = distribution.sampleNormal(u, v, &pdfM);
    Vector3 H = localH.x*DG.tangent + localH.y*DG.bitangent + localH.z*DG.normal;
    H.normalize();

    Vector3 reflDir = wo.reflect(H);
    reflDir.normalize();
    float G = distribution.computeG(wo, reflDir, H);

    m_fresnel.setNormal(H);
    float F = m_fresnel.evaluateF(wo);
    float reflProb = F;

    float cosThetaR = wo.dot(H);
    float sinThetaR = sqrt(1 - cosThetaR * cosThetaR);
    if (cosThetaR < 0 && sinThetaR > 1.0 / m_fresnel.getIOR())
    {
        // Total internal reflection
        sampleWi = reflDir;
        pdf = pdfM / (4.0f * abs(wo.dot(H)));
        //return 1.0f * G;
    }
    else if (RandomNumber::getThreadObj()->randomFloat() < reflProb)
    {
        // reflection
        sampleWi = reflDir;
        pdf = pdfM / (4.0f * abs(wo.dot(H))) * reflProb;
    }
    else
    {
        // refraction
        bool oOutSide = wo.dot(DG.normal) > 0;
        float iorO = oOutSide?1:m_fresnel.getIOR();
        float iorI = oOutSide?m_fresnel.getIOR():1;
        bool isRefract;
        sampleWi = wo.refract(H, iorO, iorI, isRefract);
        sampleWi.normalize();
        float ih = sampleWi.dot(H);
        float oh = wo.dot(H);
        float denominator = iorI * ih + iorO * oh;
        pdf = pdfM * iorO * iorO * abs(oh) / (denominator * denominator) * (1 - reflProb);
    }
    return evaluateFCos(sampleWi, wo);
}

Vector3 GGXTransparentBSDF::evaluateFCos(Vector3 & ri, const Vector3 & ro)
{
    GGXDistribution distribution(&DG, m_ag);
    float cosWi = ri.dot(DG.normal);
    float cosWo = ro.dot(DG.normal);
    bool iOutside = cosWi > 0;
    bool oOutside = cosWo > 0;
    if (cosWi > 0 && cosWo > 0)
    {
        // incoming and outgoing direction are at the same side, reflection occurs
        Vector3 rh = ri + ro;
        rh.normalize();
        m_fresnel.setNormal(rh);
        float D = distribution.computeD(rh);
        float G = distribution.computeG(ri, ro, rh);
        float F = m_fresnel.evaluateF(ri);
        float specular = D * G * F / (4 * cosWo);
        return Vector3(specular);
    }
    else if (cosWi < 0 && cosWo < 0)
    {
        // incoming and outgoing direction are at the same side, reflection occurs
        Vector3 rh = ri + ro;
        rh.normalize();
        m_fresnel.setNormal(rh * -1);
        float D = distribution.computeD(rh);
        float G = distribution.computeG(ri, ro, -1*rh);
        float F = m_fresnel.evaluateF(ri);
        float specular = D * G * F / (4 * abs(cosWo));
        return Vector3(specular);
    }
    else
    {
        // transmission occurs
        float iorI = iOutside ? 1 : m_fresnel.getIOR();
        float iorO = oOutside ? 1 : m_fresnel.getIOR();
        Vector3 rh = -1.0 * (iorI * ri + iorO * ro);
        rh.normalize();

        m_fresnel.setNormal(rh);
        float F = m_fresnel.evaluateF(ri);
        float D = distribution.computeD(rh);

        Vector3 ri_ = iOutside ? ri : ri.reflect(rh) * -1;
        Vector3 ro_ = oOutside ? ro : ro.reflect(rh) * -1;
        Vector3 rh_ = rh.dot(DG.normal) > 0 ? rh : rh * -1;
        float G = distribution.computeG(ri_, ro_, rh_);

        float ih = ri.dot(rh);
        float oh = ro.dot(rh);
        float numerator = abs(ih * oh / (cosWo)) * iorO * iorO * (1 - F) * G * D;
        float denominator = iorI * ih + iorO * oh;
        denominator *= denominator;
        float reflBRDF = D * G * (1 - F) / abs(4 * cosWo);
        Vector3 refrBRDF = numerator / denominator * m_color;
        return refrBRDF;
    }
}
