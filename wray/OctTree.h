#pragma once
//存在八叉树里面的采样点
struct WIrradianceSample
{
	WVector3 E;
	WVector3 normal;
	WVector3 point;
	float maxDist;
	//显示采样点,调试时用
	void display(bool isDisplayBox=false);
};
//插值器
//包含要进行插值计算的点和法线信息
//传入八叉树，八叉树对它进行插值处理
//结果记录在它里面
class WIrradianceInterpolator
{
public:
	//构造函数，设置各种阈值，而不是设置位置，法线信息
	//当对象构造好以后，再通过refresh函数传入位置和法线信息
	WIrradianceInterpolator(
		float imaxNormalError=0.1,
		float imaxPlanarError=0.1,
		float ismoothFactor=2,
		unsigned int iminSamples=10)
	{
		nSamplesAccepted=nSamplesChecked=0;
		totalWeight=0.0f;
		totalE=WVector3(0);
		maxNormalError=imaxNormalError;
//		maxDistanceError=imaxDistanceError;
		maxPlanarError=imaxPlanarError;
		minSamples=iminSamples;
		smoothFactor=ismoothFactor;
	}
	void setInterpolateParams(
		float imaxNormalError=0.1,
		float imaxPlanarError=0.1,
		float ismoothFactor=2.0,
		unsigned int iminSamples=10)
	{
		nSamplesAccepted=nSamplesChecked=0;
		totalWeight=0.0f;
		totalE=WVector3(0);
		maxNormalError=imaxNormalError;
//		maxDistanceError=imaxDistanceError;
		maxPlanarError=imaxPlanarError;
		minSamples=iminSamples;
		smoothFactor=ismoothFactor;
	}

	//对一个新的点进行插值
	void refresh(WVector3&iposition,WVector3&inormal)
	{
		position=iposition;normal=inormal;
		nSamplesAccepted=nSamplesChecked=0;
		totalWeight=0;
		totalE=WVector3(0);
	}
	WVector3 getPosition(){return position;}
	//计算当前点是否适合使用sample的采样点进行插值
	//如果适合，修改相关参数，并返回true
	//否则返回false
	//当所有采样点计算完毕后，
	//通过nSamplesAccepted判定是否找到合适的插值点，若找到，
	//totalE/totalWeight得到最终的E
	void operator()(WIrradianceSample&sample);
	//判断是否插值成功，并返回最终插值所得的E
	bool finalInterpolate(WVector3&E);
private:
	WVector3 normal;
	WVector3 position;

	float totalWeight;
	WVector3 totalE;

	//下面是各种阈值
	//nSamplesAccepted,nSamplesChecked分别是通过检测的样本数
	//和检测的样本总数
	unsigned int nSamplesAccepted,nSamplesChecked;
	//maxNormalError是法向量夹角余弦的阈值，越小误差越大
	//maxPlanarError是采样点共面程度的阈值，越小共面程度越大
	//smoothFactor是光照平滑过渡的阈值，越大越平滑
	float maxNormalError,maxPlanarError,smoothFactor;
	//minSamples表示最少需要的插值数
	unsigned int minSamples;
};

//八叉树节点
struct WOctNode
{
public:
	//构造函数，把子节点指针都设为NULL
	WOctNode();
public:
	vector<WIrradianceSample>samples;
	WOctNode*children[8];
};


//八叉树，用于irradianceCache方法,以帮助搜索采样点
class WOctTree
{
public:
	WOctTree(WScene*iscene,unsigned int imaxDepth=10);
	virtual ~WOctTree(void);
	void addSample(const WIrradianceSample&sample);
	//显示节点的包围盒，调试时用
	void displayNodes();
	void displaySamples(bool isDisplayBox=false);
	void initialize(WScene*iscene);

	//清除所有节点
	void clear();
	void setMaxDepth(unsigned int imaxDepth)
	{maxDepth=imaxDepth;}

	//在八叉树找到在要计算的点周围的采样点
	//并进行插值计算，结果存在IrradianceInterpolate中
	//然后通过IrradianceInterpolate里面的信息，
	//即可判断插值是否成功
	void process(WIrradianceInterpolator&interpolate);
private:
	void displaySample(WOctNode*node,bool isDisplayBox=false);
	//删除节点，此函数为递归调用
	void deleteNodes(WOctNode*node);
	//把采样点添加到八叉树合适的位置
	void add(
		WOctNode*node,
		const WIrradianceSample&sample,
		WBoundingBox nodeBox, 
		const WBoundingBox&sampleBox, 
		float diagSquared,
		unsigned int depth);
	//在八叉树找到在要计算的点周围的采样点
	//并进行插值计算，结果存在IrradianceInterpolate中
	//然后通过IrradianceInterpolate里面的信息，
	//即可判断插值是否成功
	void lookUP(
		WOctNode*node,
		WBoundingBox&nodeBox,
		WIrradianceInterpolator&interpolate
		);

	vector<WBoundingBox>boxes;			//节点包围盒，调试时用
	WOctNode*root;						//根节点
	unsigned int maxDepth;
	WScene*scene;
	WBoundingBox sceneBox;
};
