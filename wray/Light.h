#pragma once
#include "Vector3.h"
class WLight
{
public:
	enum LightType{
		LIGHT_POINT=0,
		LIGHT_RECTANGLE=1,
		LIGHT_OBJECT=2
	};
	LightType type;

	WLight(LightType itype,bool iisPoint):
	type(itype),isPoint(iisPoint){};
	virtual ~WLight(void);
	//是否点光源
	bool isPoint;

	virtual void sampleLight(
		float u1,float u2,float u3,WBSDF&bsdf,
		WVector3&iposition,WVector3&iintensity,float&PDF)=0;
	virtual void draw()=0;
	//获得属性数组,
	//此函数用于把所有灯光的属性打包到一个float4纹理中
	//所以properties的大小一定为4的倍数
	virtual void getProperties(vector<float>& properties){};
	//获得属性数组的大小
	virtual void getPropertySize(int& nFloat4s){}
};

class WPointLight:public WLight
{
public:
	WPointLight(WVector3 iintensity,WVector3 iposition):
	  WLight(LIGHT_POINT,true),intensity(iintensity),
	  position(iposition){}
	~WPointLight(){}

	void setIntensity(WVector3 iintensity)
	{intensity=iintensity;}
	void setPosition(WVector3 iposition)
	{position=iposition;}
	void sampleLight(float u1,float u2,float u3, WBSDF&bsdf,WVector3&iposition,WVector3&iintensity,float&PDF);
	void draw();
	//获得属性数组，包括灯光的强度，灯光的位置
	//属性的顺序是：强度 r g b a ， 位置 x y z w
	//a w为 1.0f
	virtual void getProperties(vector<float>& properties);
private :
	WVector3 intensity;
	WVector3 position;
};

class WRectangleLight:public WLight
{
public:
	WRectangleLight(WVector3 iposition,WVector3 idirection,
		WVector3 up,float width,float height,
		WVector3 iintensity,bool iisDoubleSide=true);
	void setIntensity(WVector3 iintensity)
	{intensity=iintensity;}
	void setPosition(WVector3 iposition)
	{position=iposition;}
	void sampleLight(float u1,float u2,float u3,WBSDF&bsdf, WVector3&iposition,WVector3&iintensity,float&PDF);
	void draw();
	//获得属性数组，包括灯光的强度，灯光的位置
	//属性的顺序是：强度 r g b a ， 位置 x y z w
	//a w为 1.0f
	virtual void getProperties(vector<float>& properties);
private:
	WVector3 position;
	WVector3 direction;
	WVector3 x,y;
	WVector3 intensity;
	float area;
	bool isDoubleSide;
};
//物体灯光
//添加物体灯光的步骤是：
//1）创建一个物体灯光对象
//2）把要发光的面加到灯光对象里面
//3）调用灯光对象的函数，修改要发光的面的材质
/*
class ObjectLight:public WLight
{
public:
	ObjectLight(WVector3 iintensity,Scene*iscene,bool iisDoubleSide):WLight(LIGHT_OBJECT,false),intensity(iintensity),scene(iscene),isDoubleSide(iisDoubleSide){}
	void addPrimitive(unsigned int nthPrimitive);
	void sampleLight(
		float u1,float u2,float u3,WBSDF&bsdf,
		WVector3&iposition,WVector3&iintensity,float&PDF);
	void draw(){};
	void clear();
private:
	Scene*scene;
	WVector3<Triangle>faces;
	WVector3 intensity;
	bool isDoubleSide;
};*/
