#pragma once
#include "Vector3.h"
#include "Ray.h"
#include "DifferentialGeometry.h"

//选择哪种办法求三角形的交点
//试验表明，方法B比较快
#define ISECTMETHOD_A 1
#define ISECTMETHOD_B 2
#define INTERSECTION_TEST_METHOD ISECTMETHOD_B
#define INTERSECTION_METHOD      ISECTMETHOD_B

#define TRIACCEL_TRIID_MASK 0x1ffffff
struct TriAccel
{
	float nu, nv, np, pu, pv, e0u, e0v, e1u, e1v;
	/* ci包含三角形的类型信息，各位分布如下
	 * 位				含义
	 * 31				是否轴对齐三角形
	 * 30 29			u 实际轴向
	 * 28 27			v 实际轴向
	 * 26 25			w 实际轴向
	 * 24- 0			三角形序号
	 */
	unsigned int ci;
	float intersectTest(const Ray&r);
};

struct WRayPacket
{
	__m128 ori[3], dir[3], tMin, tMax;
};

class WTriangle
{
private:
public:
	TriAccel tA;
	Vector3 point1,point2,point3;
	Vector2f texCoord1,texCoord2,texCoord3;
	Vector3 normal1,normal2,normal3;
    Vector3 geometricNormal;
	unsigned int mtlId;

	WTriangle(
		const Vector3&ipoint1,
		const Vector3&ipoint2,
		const Vector3&ipoint3,
		const Vector2f&itexCoord1,
		const Vector2f&itexCoord2,
		const Vector2f&itexCoord3,
		const Vector3&inormal1,
		const Vector3&inormal2,
		const Vector3&inormal3,
		unsigned int mtlId
		);

	WTriangle(void);
	virtual ~WTriangle(void);

	float area();

	//以下两个是与光线求交的函数
	//-----------------------
	//intersectTest计算相交的时候的光线t值,如果不相交，
	//返回光线的M_INF_BIG，表示在无穷远处相交
	//测试的时候只需检测返回值是否小于tMax即可
	//-----------------------
	//intersect假定已经三角形与光线必有交点，
	//求出交点的DifferentialGeometry
	//-----------------------
	//多个三角形求交的时候，先循环调用intersectTest函数
	//得到离光线最近而又相交的多边形，再调用intersect函数
	//求出交点的DifferentialGeometry,并更新光线的tMax值
	//DifferentialGeometry在创建的时候不作初始化
	//然后输入intersect函数初始化各个变量
	float intersectTest(const Ray&r);
	__m128 intersectTest(const WRayPacket& rp);
	void intersect(Ray&r,DifferentialGeometry&DG);
	//由uv参数获得表面一点及其法向量
	//用于灯光的采样
	void getPoint(float u,float v,
		Vector3&position,Vector3&normal,Vector2f&texCoord);

	//在openGL中画出三角形，调试时用
	void draw(bool showNormal=false,bool fillMode=false);
	//输出三角形数据，调试时用
	void showCoords();
	//输出三角形顶点数据，调试时用
	void showVertexCoords();
	//返回质心
	Vector3 getCentroid();
	// 
	void buildDG(float b1, float b2, const Vector3& rayDir, DifferentialGeometry& DG);
//#if INTERSECTION_METHOD==ISECTMETHOD_B
	void buildAccelerateData(int triID = 0);			// 构建加速数据结构
	TriAccel& getAccelerateData(){return tA;}
//#endif
	//__device__ TriAccel getAccelerateDataGPU(){return tA;}
};


