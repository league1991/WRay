#pragma once
//此摄像机可以设定任意像素点的颜色
class WRandomCamera:public WCamera
{
public:
	WRandomCamera(void);
	virtual ~WRandomCamera(void);
	
	//重载函数
/*
	void drawFilmInScreen(
		float offsetX =0.0f,
		float offsetY =0.0f);
	void drawFilmInWorld(
		float offsetX =0.0f,
		float offsetY =0.0f,
		float offsetZ =0.0f);	*/

	//这个方法为metropolis算法专用，
	//用来设置指定方向对应的像素点的颜色
	//方向可以由光线与场景第一个交点的坐标
	//减去摄像机观察点的坐标得到	
	void setColor(
		float R,float G,float B,
		WVector3 dir);
	//下面3个方法的功能与Camera类相同，
	//不过同时也把内置的记录像素样本采样数的数组
	//重新设置
	void setFilmResolutionX(unsigned int resX);
	void setFilmResolutionXY(
		unsigned int resX,unsigned int resY);
	void clearFilm(
		float R =0.0f,float G =0.0f,float B =0.0f);
	
	//不用这些方法
	void getNextRay(WRay&ray){}
	void setColor(float R,float G,float B){}
	void changeSampleSize(unsigned int size);	//每像素样本数始终为1
	void changeSampler(WSampler::WSamplerType type){}
	float currProgress(){return 0.0f;}
	bool isFilmFull(){return false;}
	int* getFilmBitPointer(){return NULL;}
	unsigned int getSampleSize(){return 1;}
	
private:
	unsigned int* nSamples;
	unsigned int resolutionX;
	unsigned int resolutionY;
	
};
