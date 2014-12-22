#pragma once
//此类包含各种采样方法
class WMonteCarlo
{
public:
	WMonteCarlo(void);
	virtual ~WMonteCarlo(void);
	//以下是随机采样函数，输入0-1的随机数，返回采样的坐标值
	//对圆形采样，返回单位圆内部的坐标值
	static float randomFloat();
	static void uniformSampleDisk(const float u1, const float u2, float &x, float &y);
	//对半球采样，使得采样值服从余弦分布，PDF为对应位置的概率密度函数
	static void cosineSampleHemisphere(const float u1, const float u2, WVector3 &sample,float&PDF);
	static void uniformSampleTriangle(float u1,float u2,
		float &u,float &v);
};
