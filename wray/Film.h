#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Filter.h"
//Film类储存渲染后像素点的颜色
//其拥有一个filter接口，
//filter接口负责通过采样点的颜色计算出像素点的颜色
//然后通过film的setColor函数设置Film对应像素点的颜色
class WFilm
{
public:
	WFilm(unsigned int resX=640,unsigned int resY=480,
		unsigned int sampleSize=1,
		WFilter::WFilterType filterType=WFilter::FILTER_BOX,
		WSampler::WSamplerType samplerType=WSampler::SAMPLER_RANDOM,
		unsigned int seed=65536);
	virtual ~WFilm(void);

	//设置分辨率
	void setResolution(unsigned int resX,unsigned int resY);
	WVector2 getResolution()
	{return WVector2(resolutionX,resolutionY);}


	//设置指定坐标位置的颜色，坐标原点在屏幕左下角
	WVector3 getColor(unsigned int x,unsigned int y);
	void setColor(unsigned int x,unsigned int y,float R,float G,float B);
	void accumulateColor(unsigned int x,unsigned int y,float R,float G,float B);


	//获取当前的采样点位置，位置值都在0-1之间
	//对于PathIntegrator，recursivePathIntegrator和IrradianceIntegrator,
	//此函数应该与下面的setSampleColor配合使用
	//即先调用此函数，计算出发射光线的位置
	//完成光线跟踪候把返回的颜色值输入setSampleColor
	//在一个像素的采样点都满了以后，
	//setSampleColor在必要时会同时更新这个像素点的颜色
	//对于MetropolisIntegrator，不用此种方法
	//改为调用setColor函数，直接设置像素的颜色
	void getSamplePosition(float&posX,float&posY);
	//设置采样点的颜色，必要时更新颜色数组
	void setSampleColor(float R,float G,float B);


	//openGL画图函数,x y 为所画像素的左下角坐标
	void draw(float x=0,float y=0,float z=0);
	//把像素统一设置成一种颜色
	void cleanColors(float R=1,float G=1,float B=1);

	bool isFull();//检查是否所有像素都被计算出来了
	//获得当前的像素位置


	void getCurrPos(unsigned int&icurrX,unsigned int&icurrY)
	{icurrX=currX;icurrY=currY;}
	//设置当前要渲染的像素
	void setCurrPos(unsigned int icurrX,unsigned int icurrY)
	{currX=icurrX;currY=icurrY;}


	void changeSampleSize(unsigned int size);
	void changeSampler(WSampler::WSamplerType type);

	int*getBitPointer(){return bitColors;}
	float* getFloatPointer(){return colors;}

	//继续从第一个像素开始曝光
	void nextExposurePass();

private:
	unsigned int resolutionX,resolutionY;
	float*colors;			//像素颜色数组,范围在0到1
	float*counts;			//像素计数数组，用于对结果取平均
	int*bitColors;			//像素颜色数组,范围在0到255
							//每个元素按照00RRGGBB的顺序排列
	float*currColor;		//当前颜色
	unsigned int currX,currY;//当前颜色的坐标值,左下角为0
	WFilter*filter;			//过滤器类型
	void buildColorArray();
	int float2int(float&r,float&g,float&b);		//此函数把浮点型的颜色值转换成整形
};
