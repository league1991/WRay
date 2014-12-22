#pragma once
#include "Vector3.h"
#include "Film.h"
class WCamera
{
public:
	WCamera(void);
	virtual ~WCamera(void);
	void setParameter(
		WVector3 iori=WVector3(0,0,1),
		WVector3 itar=WVector3(0,0,0),
		WVector3 iup=WVector3(0,0,1),
		float ifov= 3.14159265/3.0f,
		float iratio=1.33f
		);
	void setDirectionParams(
		WVector3 iori=WVector3(0,0,1),
		WVector3 itar=WVector3(0,0,0),
		WVector3 iup=WVector3(0,0,1));

	//设置胶片分辨率，竖直方向分辨率根据宽高比算出
	virtual void setFilmResolutionX(unsigned int resX);
	virtual void setFilmResolutionXY
		(unsigned int resX,unsigned int resY);
	virtual void getFilmResolution(int&resX, int&resY);
	//画出摄像机的线框
	virtual void drawCamera(float R=0,float G=0,float B=0.8);
	//画出底片内容，实际上是渲染完毕的图像
	//offsetX=0.0,offsetY=0.0f时，图像画在摄像机左下角
	virtual void drawFilmInScreen(float offsetX=0.0f,float offsetY=0.0f);
	//也是画出底片内容，所不同的是需要输入世界坐标值
	virtual void drawFilmInWorld(float offsetX=0.0f,
		float offsetY=0.0f,float offsetZ=0.0f);
	//产生下条光线
	void getNextRay(WRay&ray);
	void getNextRay( WRay&ray, float xi, float yi );
	//设置光线追踪之后返回的颜色
	void setColor(float R,float G,float B);
	void setColor(float R, float G, float B, int x, int y);
	void accumulateColor( float R, float G, float B, int x, int y );
	WVector3 getColor(unsigned int x,unsigned int y);
	//检查是不是所有像素都被设置了
	virtual bool isFilmFull();
	//表示重新开始渲染,会重新设置底片颜色
	virtual void clearFilm(float R=0.0,float G=0.0,float B=0.0);
	//继续从第一个像素开始曝光,之前的颜色会被叠加进去
	virtual void nextExposurePass();
	virtual void changeSampleSize(unsigned int size);
	virtual void changeSampler(WSampler::WSamplerType type);
	//返回当前渲染进度
	virtual float currProgress();
	virtual WVector3 getDirection(){return dir;}
	virtual WVector3 getOrigin(){return origin;}
	virtual void getXY(WVector3&ix,WVector3&iy)
	{ix=x;iy=y;	}
	virtual int*getFilmBitPointer();
	virtual unsigned int getSampleSize(){return filmSampleSize;};
	virtual float getRatio(){return ratio;}

protected:
	WVector3 origin;		//起点
	WVector3 target;		//目标点	
	WVector3 dir;		//由起点指向目标点的单位向量，长度为1
	WVector3 up;			//向上向量
	WVector3 x,y;		//屏幕x y坐标的空间方向,不是单位向量
						//屏幕跟起点的距离为1
	float fov;
	float ratio;		//宽高比（宽/高）
	WFilm film;
	unsigned int filmSampleSize;

	//把胶片都设置成同一种颜色
	void cleanFilmColors(float R=1.0,float G=1.0,float B=1.0);
	//通过屏幕的xy坐标产生一条光线
	//xRatio，yRatio的值均在-1到1之间	
	//计算xy向量
	void computeXY();
	void setFilmResolutionX(unsigned int resX,unsigned int resY);

private:
	void generateRay(float xRatio,float yRatio,WRay&ray);
};
