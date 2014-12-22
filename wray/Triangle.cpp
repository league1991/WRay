#include "stdafx.h"
#include "MathValues.h"
#include "Vector3.h"
#include "Triangle.h"
#include <gl/glut.h>

float TriAccel::intersectTest( const WRay&r )
{
	char cii = ci >> 25;
	char w = cii & 0x3;	cii >>= 2;
	char v = cii & 0x3;	cii >>= 2;
	char u = cii & 0x3;
	float det, dett;
	if (cii & 0x4)			// 轴对齐三角形
	{
		det  = r.direction.v[w];
		dett = np - r.point.v[w];
	}
	else
	{
		det = r.direction.v[u] * nu + r.direction.v[v] * nv + r.direction.v[w];
		dett = np - (r.point.v[u] * nu + r.point.v[v] * nv+ r.point.v[w]);
	}
	float Du = r.direction.v[u] * dett - (pu - r.point.v[u]) * det;
	float Dv = r.direction.v[v] * dett - (pv - r.point.v[v]) * det;
	float detu = e1v * Du - e1u * Dv;
	float detv = e0u * Dv - e0v * Du;

	float tmpdet0 = det - detu - detv;
	int* pdet0 = reinterpret_cast<int*>(&tmpdet0);
	int* pdetu = reinterpret_cast<int*>(&detu);
	int* pdetv = reinterpret_cast<int*>(&detv);
	*pdet0 = *pdet0 ^ *pdetu;
	int tmpdet1 = *pdetu ^ *pdetv;
	*pdet0 = (*pdet0 | tmpdet1);
	if (*pdet0 & 0x80000000)
		return M_INF_BIG;
	float t = dett / det;
	if((t<r.tMin) | (t>r.tMax))
		return M_INF_BIG;
	return t;
}

WTriangle::WTriangle(void)
{
}

WTriangle::WTriangle(
		 const WVector3&ipoint1,
		 const WVector3&ipoint2,
		 const WVector3&ipoint3,
		 const WVector2&itexCoord1,
		 const WVector2&itexCoord2,
		 const WVector2&itexCoord3,
		 const WVector3&inormal1,
		 const WVector3&inormal2,
		 const WVector3&inormal3,
		 unsigned int imtlId
		 ):point1(ipoint1),point2(ipoint2),point3(ipoint3),
		 texCoord1(itexCoord1),texCoord2(itexCoord2),texCoord3(itexCoord3),
		 normal1(inormal1),normal2(inormal2),normal3(inormal3),
		 mtlId(imtlId)
{}

WTriangle::~WTriangle(void)
{
}
float WTriangle::area()
{
	WVector3 p21=point2-point1;
	WVector3 p31=point3-point1;
	return abs(0.5f*p21.cross(p31).length());
}
//求交函数,注意光线的方向向量必须事先单位化
float WTriangle::intersectTest(const WRay&r)
{
#if INTERSECTION_TEST_METHOD == ISECTMETHOD_A
	WVector3 E1=point2-point1;
	WVector3 E2=point3-point1;
	WVector3 T=r.point-point1;
	WVector3 S1=r.direction.cross(E2);
	WVector3 S2=T.cross(E1);
	float S1E1=S1.dot(E1);

	if(abs(S1E1)<1e-9f)		//认为分母为0 ，故求交失败
		return M_INF_BIG;

	float t=S2.dot(E2)/S1E1;
	if(t<r.tMin||t>r.tMax)	
		//t超出光线范围，求交失败,注意当t=tMax的时候有可能求交成功
		return M_INF_BIG;

	float b1=S1.dot(T)/S1E1;
	if(b1<0.0f)		//b1超出范围，求交失败
		return M_INF_BIG;

	float b2=S2.dot(r.direction)/S1E1;
	if(b2<0.0f)		//b2超出范围，求交失败
		return M_INF_BIG;
	if(b1+b2>1.0f)
		return M_INF_BIG;
	//至此求交成功
	return t;
#else
	unsigned ci = tA.ci >> 25;
	char w = ci & 0x3;	ci >>= 2;
	char v = ci & 0x3;	ci >>= 2;
	char u = ci & 0x3;
	float det, dett;
	if (ci & 0x4)			// 轴对齐三角形
	{
		det  = r.direction.v[w];
		dett = tA.np - r.point.v[w];
	}
	else
	{
		det = r.direction.v[u] * tA.nu + 
			r.direction.v[v] * tA.nv +
			r.direction.v[w];
		dett = tA.np - (r.point.v[u] * tA.nu + 
			r.point.v[v] * tA.nv+ r.point.v[w]);
	}
	float Du = r.direction.v[u] * dett - (tA.pu - r.point.v[u]) * det;
	float Dv = r.direction.v[v] * dett - (tA.pv - r.point.v[v]) * det;
	float detu = tA.e1v * Du - tA.e1u * Dv;
	float detv = tA.e0u * Dv - tA.e0v * Du;

	float tmpdet0 = det - detu - detv;
	int* pdet0 = reinterpret_cast<int*>(&tmpdet0);
	int* pdetu = reinterpret_cast<int*>(&detu);
	int* pdetv = reinterpret_cast<int*>(&detv);
	*pdet0 = *pdet0 ^ *pdetu;
	int tmpdet1 = *pdetu ^ *pdetv;
	*pdet0 = (*pdet0 | tmpdet1);
	if (*pdet0 & 0x80000000)
		return M_INF_BIG;
	float t = dett / det;
	if((t<r.tMin) || (t>r.tMax))
		return M_INF_BIG;
	return t;
#endif
}
void WTriangle::intersect(WRay&r,WDifferentialGeometry&DG)
{

#if INTERSECTION_METHOD == ISECTMETHOD_A
	WVector3 E1=point2-point1;
	WVector3 E2=point3-point1;
	WVector3 T=r.point-point1;
	WVector3 S1=r.direction.cross(E2);
	WVector3 S2=T.cross(E1);
	float S1E1=S1.dot(E1);
	float t=S2.dot(E2)/S1E1;
	float b1=S1.dot(T)/S1E1;
	float b2=S2.dot(r.direction)/S1E1;
	r.tMax=t;
	DG.position=(1.0f-b1-b2)*point1+b1*point2+b2*point3;
	DG.normal=(1.0f-b1-b2)*normal1+b1*normal2+b2*normal3;
	DG.texCoord=(1.0f-b1-b2)*texCoord1+b1*texCoord2+b2*texCoord3;
	DG.normal.normalize();
	DG.mtlId=this->mtlId;

	//算出副切线
	DG.bitangent=DG.normal.cross(DG.rayDir);
	if(DG.bitangent.lengthSquared()<1e-4f)
	{
		//认为是零向量，此时normal和rayDir或者共线，或者至少一个为0
		if(abs(DG.normal.x)>abs(DG.normal.y))
		{
			DG.bitangent=WVector3(-DG.normal.z,0.0f,DG.normal.x);
		}
		else
		{
			DG.bitangent=WVector3(0.0f,DG.normal.z,-DG.normal.y);
		}
	}
	//算出主切线
	DG.bitangent.normalize();
	DG.tangent=DG.bitangent.cross(DG.normal);
	DG.rayDir=-1*r.direction;

	//算出dpdu dpdv dndu dndv
	float a=texCoord3.x-texCoord1.x;
	float b=texCoord3.y-texCoord1.y;
	float c=texCoord2.x-texCoord1.x;
	float d=texCoord2.y-texCoord1.y;
	float det=a*d-b*c;
	if(abs(det)>1e-5f)
	{/*
		float invDet=1.0f/det;
		DG.dpdu=(d*E2-b*E1)*invDet;
		DG.dpdv=(a*E1-c*E2)*invDet;
		WVector3 N1=normal2-normal1;
		WVector3 N2=normal3-normal1;
		DG.dndu=(d*N2-b*N1)*invDet;
		DG.dndv=(a*N1-c*N2)*invDet;*/
	}
	else
	{
		DG.dpdu=DG.dpdv=DG.dndu=DG.dndv=WVector3(0.0f,0.0f,0.0f);
	}
#else
	unsigned ci = tA.ci >> 25;
	unsigned char w = ci & 0x3;	ci >>= 2;
	unsigned char v = ci & 0x3;	ci >>= 2;
	unsigned char u = ci & 0x3;
	float det, dett;
	if (ci & 0x4)			// 轴对齐三角形
	{
		det  = r.direction.v[w];
		dett = tA.np - r.point.v[w];
	}
	else
	{
		det = r.direction.v[u] * tA.nu + 
			r.direction.v[v] * tA.nv +
			r.direction.v[w];
		dett = tA.np - (r.point.v[u] * tA.nu + 
			r.point.v[v] * tA.nv+ r.point.v[w]);
	}
	float Du = r.direction.v[u] * dett - (tA.pu - r.point.v[u]) * det;
	float Dv = r.direction.v[v] * dett - (tA.pv - r.point.v[v]) * det;
	float detu = tA.e1v * Du - tA.e1u * Dv;
	float detv = tA.e0u * Dv - tA.e0v * Du;
	float rdet = 1.0f / det;
	float t = dett * rdet;
	float b1= detu * rdet;
	float b2= detv * rdet;
	r.tMax=t;
	float b3= 1.0f-b1-b2;
	DG.position=b3*point1+b1*point2+b2*point3;
	DG.normal=b3*normal1+b1*normal2+b2*normal3;
	DG.texCoord=b3*texCoord1+b1*texCoord2+b2*texCoord3;
	DG.normal.normalize();
	DG.mtlId=this->mtlId;

	//算出副切线
	DG.bitangent=DG.normal.cross(DG.rayDir);
	if(DG.bitangent.lengthSquared()<1e-4f)
	{
		//认为是零向量，此时normal和rayDir或者共线，或者至少一个为0
		if(abs(DG.normal.x)>abs(DG.normal.y))
		{
			DG.bitangent=WVector3(-DG.normal.z,0.0f,DG.normal.x);
		}
		else
		{
			DG.bitangent=WVector3(0.0f,DG.normal.z,-DG.normal.y);
		}
	}
	//算出主切线
	DG.bitangent.normalize();
	DG.tangent=DG.bitangent.cross(DG.normal);
	DG.rayDir=-1*r.direction;

	//算出dpdu dpdv dndu dndv
	float a=texCoord3.x-texCoord1.x;
	float b=texCoord3.y-texCoord1.y;
	float c=texCoord2.x-texCoord1.x;
	float d=texCoord2.y-texCoord1.y;
	det=a*d-b*c;
	if(abs(det)>1e-5f)
	{/*
		float invDet=1.0f/det;
		DG.dpdu=(d*E2-b*E1)*invDet;
		DG.dpdv=(a*E1-c*E2)*invDet;
		WVector3 N1=normal2-normal1;
		WVector3 N2=normal3-normal1;
		DG.dndu=(d*N2-b*N1)*invDet;
		DG.dndv=(a*N1-c*N2)*invDet;*/
	}
	else
	{
		DG.dpdu=DG.dpdv=DG.dndu=DG.dndv=WVector3(0.0f,0.0f,0.0f);
	}
#endif
}


void WTriangle::getPoint(float b1,float b2,
						WVector3&position,
						WVector3&normal,
						WVector2&texCoord)
{
	position=(1.0f-b1-b2)*point1+b1*point2+b2*point3;
	normal=(1.0f-b1-b2)*normal1+b1*normal2+b2*normal3;
	texCoord=(1.0f-b1-b2)*texCoord1+b1*texCoord2+b2*texCoord3;
}
void WTriangle::draw(bool showNormal,bool fillMode)
{
	if(fillMode)
		glPolygonMode(GL_FRONT,GL_FILL);
	glBegin(GL_TRIANGLES);
//	glColor3f(texCoord1.x,texCoord1.y,0.2f);
	glVertex3f(point1.x,point1.y,point1.z);
//	glColor3f(texCoord2.x,texCoord2.y,0.2f);
	glVertex3f(point2.x,point2.y,point2.z);
//	glColor3f(texCoord3.x,texCoord3.y,0.2f);
	glVertex3f(point3.x,point3.y,point3.z);
	glEnd();
	if(showNormal)
	{
		WVector3 norEnd1,norEnd2,norEnd3;
		norEnd1=point1+normal1*0.2f;
		norEnd2=point2+normal2*0.2f;
		norEnd3=point3+normal3*0.2f;
		glBegin(GL_LINES);
		glVertex3f(point1.x,point1.y,point1.z);
		glVertex3f(norEnd1.x,norEnd1.y,norEnd1.z);
		glVertex3f(point2.x,point2.y,point2.z);
		glVertex3f(norEnd2.x,norEnd2.y,norEnd2.z);
		glVertex3f(point3.x,point3.y,point3.z);
		glVertex3f(norEnd3.x,norEnd3.y,norEnd3.z);
		glEnd();
	}
}
void WTriangle::showCoords()
{
	cout<<"#####WTriangle#####"<<endl;
	cout<<"vertices"<<endl;
	point1.showCoords();
	point2.showCoords();
	point3.showCoords();
	cout<<"normals"<<endl;
	normal1.showCoords();
	normal2.showCoords();
	normal3.showCoords();
	cout<<"texcoords"<<endl;
	texCoord1.showCoords();
	texCoord2.showCoords();
	texCoord3.showCoords();
	cout<<"mtlID"<<mtlId<<endl;
}
WVector3 WTriangle::getCentroid()
{
	return (point1+point2+point3)/3.0f;
}

void WTriangle::showVertexCoords()
{
	cout<<"vertices"<<endl;
	point1.showCoords();
	point2.showCoords();
	point3.showCoords();
}

//#if INTERSECTION_METHOD==ISECTMETHOD_B
void WTriangle::buildAccelerateData(int triID)
{
	WVector3 normal = (point2 - point1).cross(point3 - point1);
	int u = 0, v = 1, w = 2;
	float maxnw = fabs(normal.z);
	float nw = normal.z;
	if (fabs(normal.y) > maxnw){
		w = 1; u = 0; v = 2;
		nw = normal.y;
		maxnw = fabs(normal.y);
	}
	if (fabs(normal.x) > maxnw){
		w = 0; u = 1; v = 2;
		nw = normal.x;
		maxnw = fabs(normal.x);
	}
	tA.nu = normal.v[u] / nw;
	tA.nv = normal.v[v] / nw;
	tA.pu = point1.v[u];
	tA.pv = point1.v[v];
	tA.np = tA.nu * tA.pu + tA.nv * tA.pv + point1.v[w];
	WVector3 e0 = point2 - point1;
	WVector3 e1 = point3 - point1;
	float f = (w == 1) ? -1.0f : 1.0f;
	tA.e0u = f * e0.v[u] / (nw);
	tA.e0v = f * e0.v[v] / (nw);
	tA.e1u = f * e1.v[u] / (nw);
	tA.e1v = f * e1.v[v] / (nw);
	tA.ci = (u << 29) | (v << 27) | (w << 25) | triID;
	if (tA.nu == 0.0f && tA.nv == 0.0f)
	{
		tA.ci |= (0x1 << 31);
	}
}
//#endif

__m128 WTriangle::intersectTest(const WRayPacket& rp)
{
	char ci = tA.ci >> 25;
	char w = ci & 0x3;
	ci >>= 2;
	char v = ci & 0x3;
	ci >>= 2;
	char u = ci & 0x3;
	__m128 det, dett;
	if (ci & 0x40)			// 轴对齐三角形
	{
		det  = rp.dir[w];
		dett = _mm_sub_ps(_mm_set_ps1(tA.np), rp.ori[w]);
	}
	else
	{
		__m128 dunu = _mm_mul_ps(rp.dir[u], _mm_set_ps1(tA.nu));
		__m128 dvnv = _mm_mul_ps(rp.dir[v], _mm_set_ps1(tA.nv));
		det  = _mm_add_ps(dunu, dvnv);
		det = _mm_add_ps(det, rp.dir[w]);

		__m128 ounu = _mm_mul_ps(rp.ori[u], _mm_set_ps1(tA.nu));
		__m128 ovnv = _mm_mul_ps(rp.ori[v], _mm_set_ps1(tA.nv));
		dett = _mm_add_ps(ounu, ovnv);
		dett = _mm_add_ps(dett, rp.ori[w]);
		dett = _mm_sub_ps(_mm_set_ps1(tA.np), dett);
	}

	__m128 puou = _mm_sub_ps(_mm_set_ps1(tA.pu), rp.ori[u]);
	puou = _mm_mul_ps(puou, det);
	__m128 Du = _mm_mul_ps(rp.dir[u], dett);
	Du = _mm_sub_ps(Du, puou);

	__m128 pvov = _mm_sub_ps(_mm_set_ps1(tA.pv), rp.ori[v]);
	pvov = _mm_mul_ps(pvov, det);
	__m128 Dv = _mm_mul_ps(rp.dir[v], dett);
	Dv = _mm_sub_ps(Dv, pvov);

	__m128 e0vDu = _mm_mul_ps(_mm_set_ps1(tA.e0v), Du);
	__m128 e0uDv = _mm_mul_ps(_mm_set_ps1(tA.e0u), Dv);
	__m128 e1vDu = _mm_mul_ps(_mm_set_ps1(tA.e1v), Du);
	__m128 e1uDv = _mm_mul_ps(_mm_set_ps1(tA.e1u), Dv);

	__m128 detu = _mm_sub_ps(e1vDu, e1uDv);
	__m128 detv = _mm_sub_ps(e0uDv, e0vDu);

	__m128 tmpdet0 = _mm_sub_ps(det, _mm_add_ps(detu, detv));
	__m128 det0   = _mm_xor_ps(tmpdet0, detu);
	__m128 tmpdet1 = _mm_xor_ps(detu, detv);

	// 如果无交点，isHit 为 0xffffffff， 否则为 0 
	__m128 isHit   = _mm_or_ps(det0, tmpdet1);	
	isHit = _mm_castsi128_ps(_mm_srai_epi32(_mm_castps_si128(isHit), 31));
	__m128 t    = _mm_div_ps(dett, det);
	isHit = _mm_or_ps(isHit, _mm_cmplt_ps(t, rp.tMin));
	isHit = _mm_or_ps(isHit, _mm_cmpgt_ps(t, rp.tMax));

	// 如果有交点，返回t值，否则返回 0xffffffff
	return _mm_or_ps(isHit, t);
}

