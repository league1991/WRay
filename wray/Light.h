#pragma once
#include "Vector3.h"
class Light
{
public:
	enum LightType{
		LIGHT_POINT=0,
		LIGHT_RECTANGLE=1,
		LIGHT_OBJECT=2
	};
	LightType type;

	Light(LightType itype,bool iisPoint):
	type(itype),isPoint(iisPoint){};
	virtual ~Light(void);
	//是否点光源
	bool isPoint;

	virtual void sampleLight(
		float u1,float u2,float u3,BSDF&bsdf,
		Vector3&iposition,Vector3&iintensity,float&PDF)=0;
	virtual void draw()=0;
	//获得属性数组,
	//此函数用于把所有灯光的属性打包到一个float4纹理中
	//所以properties的大小一定为4的倍数
	virtual void getProperties(vector<float>& properties){};
	//获得属性数组的大小
	virtual void getPropertySize(int& nFloat4s){}
	virtual bool isDeltaLight() = 0;
};

class PointLight:public Light
{
public:
	PointLight(Vector3 iintensity,Vector3 iposition):
	  Light(LIGHT_POINT,true),intensity(iintensity),
	  position(iposition){}
	~PointLight(){}

	void setIntensity(Vector3 iintensity)
	{intensity=iintensity;}
	void setPosition(Vector3 iposition)
	{position=iposition;}
	void sampleLight(float u1,float u2,float u3, BSDF&bsdf,Vector3&iposition,Vector3&iintensity,float&PDF);
	void draw();
	//获得属性数组，包括灯光的强度，灯光的位置
	//属性的顺序是：强度 r g b a ， 位置 x y z w
	//a w为 1.0f
	virtual void getProperties(vector<float>& properties);
	virtual bool isDeltaLight() { return true; }
private :
	Vector3 intensity;
	Vector3 position;
};

class RectangleLight:public Light
{
public:
	RectangleLight(Vector3 iposition,Vector3 idirection,
		Vector3 up,float width,float height,
		Vector3 iintensity,bool iisDoubleSide=true);
	void setIntensity(Vector3 iintensity)
	{intensity=iintensity;}
	void setPosition(Vector3 iposition)
	{position=iposition;}
	void sampleLight(float u1,float u2,float u3,BSDF&bsdf, Vector3&iposition,Vector3&iintensity,float&PDF);
	void draw();
	//获得属性数组，包括灯光的强度，灯光的位置
	//属性的顺序是：强度 r g b a ， 位置 x y z w
	//a w为 1.0f
	virtual void getProperties(vector<float>& properties);
	virtual bool isDeltaLight() { return false; }
private:
	Vector3 position;
	Vector3 direction;
	Vector3 x,y;
	Vector3 intensity;
	float area;
	bool isDoubleSide;
};
//物体灯光
//添加物体灯光的步骤是：
//1）创建一个物体灯光对象
//2）把要发光的面加到灯光对象里面
//3）调用灯光对象的函数，修改要发光的面的材质

class ObjectLight:public Light
{
public:
	ObjectLight(Vector3 iintensity, int materialID, Scene*iscene,bool iisDoubleSide):
		Light(LIGHT_OBJECT,false), m_materialID(materialID), m_intensity(iintensity),m_scene(iscene),m_isDoubleSide(iisDoubleSide)
	{}

	void addTriangle(int objectID, int triangleID);

	void sampleLight(
		float u1, float u2, float u3, BSDF&bsdf,
		Vector3&iposition, Vector3&iintensity, float&PDF);
	void draw(){};
	void clear() { m_faceIDList.clear(); }
	virtual bool isDeltaLight() { return false; }
private:
	Scene*m_scene;
	struct FaceID
	{
		int m_objectID;
		int m_triangleID;
	};
    std::vector<FaceID> m_faceIDList; // object id -> face id

	Vector3 m_intensity;

	bool m_isDoubleSide;
	int m_materialID;
};
