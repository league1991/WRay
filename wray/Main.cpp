
#include "stdafx.h"

GLint winWidth=800,winHeight=600;
WObjReader reader;


float alpha=M_PI_2*3.0f,beta=0.0f;
float r=10;  //这个变量用于与alpha, beta协同确定Lx, Ly, Lz（也就是相机位置）的值
float fovDeg = 60;  //某角度你懂的
float fov = fovDeg / 180.0f * M_PI;  //某角度的rad制表示
float Lx,Ly,Lz;

/**********鼠标控制需要使用的变量，目前感觉它并没有实际生效*******************/
float lastMouseX,lastMouseY;  
bool isLeftButtonDown;
bool isMiddleButtonDown;
bool isRightButtonDown;
/**********鼠标控制变量End*********************/

/*************键盘控制变量******************/
bool isFirstMove;
bool isAltKeyDown;

//用于控制是否显示左下角小方框
bool isShowScene = true;
//用于控制是否显示左下角方框中的坐标轴
bool isShowGrid = true;
/*************键盘控制变量End***************/

WVector3 cameraTarget(0);
WRay myRay;

//WRay myRay2;
float rayalpha = 0.1f,raybeta = 0.1f;
//WDifferentialGeometry DG;
WScene myScene;
WAccelerator* myAccelerator;
//WOctTree*myOctTree;
WCamera*myCamera;
WLight *myLight;
WLight *myLight2;
//WSurfaceIntegrator*myIntegrator;
//Film myFilm(200,200,1,Filter::FILTER_BOX,Sampler::SAMPLER_RANDOM,65536);
bool isComputing=false;


bool isEyeChanged = false;
//#define PRODUCTION_RENDER
//#define PACKET_INTERSECT
int numCores = 4;//这个变量用于设定线程数,可以适当增加线程数来提高渲染效率 
int threadIDArray[8] = {0,1,2,3,4,5,6,7};
//clock_t renderTime[8];
int isThreadComputing[8];

bool isInitiate = true;

//重要的地方在init()函数与compute函数与上面的那堆变量声明


#define INFINITE_RENDER
DWORD WINAPI compute(LPVOID lpParam)  //使用工程中的某些功能来进行光线跟踪计算并绘制计算结果
{
	int threadID = *((int*)lpParam);
	//cout << "threadID" << threadID << endl;
	isThreadComputing[threadID] = 1;
	if(/*isComputing==true*/true)
	{
//		if (isEyeChanged)
//		{
			myCamera->clearFilm();
//			isEyeChanged = false;
//		}

//		float progress=0,newprogress;
//		renderTime[threadID] = clock();
		WVector3 color;
		//光线跟踪
		WRay r;
		//把统计量全部清零
		myAccelerator->resetStatistics();

		int resX, resY;
		//取得窗口分辨率
		//如果要移植， 这里的myCamera得换成目标平台所设定的Camera对象，或许还得另外为它加一个获取分辨率的方法
		//或者就沿用这个项目里面的Camera类，用目标平台所维护的对象来初始化这个Camera，下面的Scene类同此。
		myCamera->getFilmResolution(resX, resY);


		//非活动预处理块，不知何用，可能没用
//#ifdef PACKET_INTERSECT
//		const int groupSize = 32767;
//		WRayGroup rayGroup;
//		rayGroup.allocSpace(groupSize);
//
//		int nGroups = int(float(resX * resY) / groupSize + 0.5);
//		int nthRay = 0;
//		for (int ithGroup = 0; ithGroup < nGroups; ++ithGroup)
//		{
//			int nRays = 0;
//			for (nthRay = ithGroup * groupSize; 
//				nthRay < (ithGroup + 1) * groupSize && nthRay < resX * resY; nthRay++, nRays++)
//			{
//				int x = nthRay % resX;
//				int y = nthRay / resX;
//				WRay r;
//				myCamera->getNextRay(r, x, y);
//				rayGroup.ori_min[nRays * 4] = r.point.x;
//				rayGroup.ori_min[nRays * 4 + 1] = r.point.y;
//				rayGroup.ori_min[nRays * 4 + 2] = r.point.z;
//				rayGroup.ori_min[nRays * 4 + 3] = r.tMin;
//				rayGroup.dir_max[nRays * 4] = r.direction.x;
//				rayGroup.dir_max[nRays * 4 + 1] = r.direction.y;
//				rayGroup.dir_max[nRays * 4 + 2] = r.direction.z;
//				rayGroup.dir_max[nRays * 4 + 3] = r.tMax;
//				rayGroup.ori[0][nRays] = r.point.x;
//				rayGroup.ori[1][nRays] = r.point.y;
//				rayGroup.ori[2][nRays] = r.point.z;
//				rayGroup.invDir[0][nRays] = 1.0f / r.direction.x;
//				rayGroup.invDir[1][nRays] = 1.0f / r.direction.y;
//				rayGroup.invDir[2][nRays] = 1.0f / r.direction.z;
//				rayGroup.isectTriangle[nRays] = NULL;
//			}
//			rayGroup.numRays = nRays;
//			WMultiRSBVH& rsBVH = *((WMultiRSBVH*)myAccelerator);
//			rsBVH.intersect(rayGroup);
//			for (nthRay = ithGroup * groupSize, nRays = 0;
//				nthRay < (ithGroup + 1) * groupSize && nthRay < resX * resY; 
//				nthRay++, nRays++)
//			{
//				int x = nthRay % resX;
//				int y = nthRay / resX;
//				WVector3 color;
//				if (rayGroup.isectTriangle[nRays])
//					color = rayGroup.DG[nRays].normal;
//				else
//					color = WVector3(0);
//				myCamera->setColor(fabs(color.x),fabs(color.y),fabs(color.z),x,y);
//			}
//			glutPostRedisplay();
//		}
//#else
		//这里传入的参数，第一个的myScene可能有点麻烦，可能需要换成目标平台的相应Scene对象，并且需要做一些修改
		WPathIntegrator * threadIntegrator=new WPathIntegrator(&myScene,myAccelerator,20,WSampler::SAMPLER_RANDOM,1.0f);
		//yInterval变量存储的是图像按y轴分割成块,每块的厚度,本程序中numCores的值为4
		int yInterval = resY / numCores;
		//根据线ID(本程序内自定)决定y的起始值
		int yBegin    = yInterval * threadID;
		int ithIteration = 0;    //迭代次数计数
		for (ithIteration = 0; ithIteration < 10000; ++ithIteration)
		{
			//扫描..先行后列
			for (int y = yBegin;  y < yBegin + yInterval; ++y)
			{
				for (int x = 0; x < resX; ++x) //resX是x轴方向分辨率,换句话说也就是这里的x的最大可以取到的值 
				{
//#ifdef PRODUCTION_RENDER
					color = WVector3(0,0,0);
					const int dimSubSample = 1;  //方形像素点，每条边分成dimSubSample份，也就是把一个像素点分成dimSubSample^2份，有抗锯齿的效果
					const float subSampleSize = 1.0f / float(dimSubSample);//此处可设置采样点的大小,采样点大小对图像效果的影响暂不明确
					for (int subY = 0; subY < dimSubSample; ++subY)
					{
						for (int subX = 0; subX < dimSubSample; ++subX)
						{
							float rayX = x + subX * subSampleSize;
							float rayY = y + subY * subSampleSize;
							//r初次传入时是以方向为0,0,0传入的
							myCamera->getNextRay(r, rayX, rayY);
							color+=threadIntegrator->integrate(r);
						}
					}
					const float numSubSample = dimSubSample * dimSubSample;
					//下面的accumulateColor函数的作用是将计算得到的颜色累加到它类中的颜色数据成员中.
					//这个函数并没有做绘制的工作,它只是计算得到最终颜色结果并存储之,然后在display函数中绘制它
					myCamera->accumulateColor(color.x / numSubSample,color.y / numSubSample,color.z / numSubSample, x, y);
//					printf("color.r=%f, color.g=%f, color.b=%f\n", color.x, color.y, color.z);
					//myCamera->accumulateColor(0.5,0.8,1.0, x, y);
//#else
//					WDifferentialGeometry DG;
//					myAccelerator->intersect(r, DG, NULL, -1);
//					myCamera->setColor(abs(DG.normal.x),abs(DG.normal.y),abs(DG.normal.z), x, y);
//#endif
				}
				if (isComputing == false)
					goto END_RENDERING;
/*
				newprogress= float(y) / resY;
				if(int(progress*20)!=int(newprogress*20))
				{
					progress=newprogress;
					cout<<"rendering progress"<<int(100*progress)<<'%'<<'\r';
				}
*/
			}
			//cout << threadID <<"th thread\t" << ithIteration << " th Iteration" << endl;
		}
//#endif


//结束绘制，回收资源
END_RENDERING:
		delete threadIntegrator;
		glutPostRedisplay();
		//myAccelerator->displayIsectStatistics();
		//myIntegrator->displayTime();
//		renderTime[threadID] =clock()-renderTime[threadID] ;
//		cout<<"thread " << threadID << " rendering time:"
//			<<double(renderTime[threadID])/CLOCKS_PER_SEC
//			<<'s'<<endl;
//		cout<<endl;
	}
//	isThreadComputing[threadID] = false;
//	for (;;)
//	{
//		if (isThreadComputing[0] + 
//			isThreadComputing[1] + 
//			isThreadComputing[2] + 
//			isThreadComputing[3] == 0)
//		{
//			isComputing = false;
//			break;
//		}
//		Sleep(250);
//	}
	return 0;
}



void init()
{
	glClearColor(0.5,0.5,0.5,0.5);
	glEnableClientState(GL_VERTEX_ARRAY);

	//initialize camera
	//相机的位置，由r, beta, alpha三个值共同确定。
	Lx=r*cos(beta)*cos(alpha);
	Ly=r*cos(beta)*sin(alpha);
	Lz=r*sin(beta);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Lx,Ly,Lz,20,20,20,0,0,1);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovDeg,float(winWidth)/float(winHeight), 0.1f,1000.0f);

	reader.readFile("areaLight.obj");
	//设置光线，具体用法还不确定，似乎没起到作用
	//myRay.point=WVector3(0,0,0);
	//myRay.direction=WVector3(1.01,0.01,0.01);
	//myRay.tMin=1e-7f;
	//rayalpha=raybeta=0.0f;


	//myScene根据reader的数据构建需要渲染的场景
	//移植到目标平台时可以考虑利用目标平台已经读入的数据来构建一个可以让myScene使用的reader对象

	//这里的buildScene(reader), 更改成buildScene(Mesh);应该就OK，前面的reader.readFile()可以去掉
	myScene.buildScene(reader);
//	myScene.rebuildAllSubPs(1);
	//myScene.showMaterials();
	//myScene.showTriangles();
	//将数据移交到myScene之后就将reader中的数据清空了。
	//这句也可以不用了
	reader.clear();

//多种加速器,可以在下面任选一种,最终效果一样,但在各方面的效率有所不同
//	myAccelerator = new WSimpleBVH();
	myAccelerator = new WMultiBVH();
//	myAccelerator = new WSimpleKD();
//	myAccelerator = new WSimpleKD2();
//	myAccelerator = new WEnhancedKD();
//	myAccelerator = new WDualKD();


	//clock 似乎只是用来算建立加速树所用的时间的。
//	clock_t c = clock();
	myAccelerator->buildTree(myScene);
//	printf("\nbuild time:%fs\n", (clock() - c) / (float)CLOCKS_PER_SEC);
//	myAccelerator->displayNodeInfo();


	//同样是无用的。
	/*WSimpleKD* skd = (WSimpleKD*)myAccelerator;
	WSKDNodeGPU* iGPU;
	WSKDLeafBodyGPU* lGPU;
	int* tGPU;
	int numNode, numLeafBody, numTriArray;*/
//	skd->getGPUArray(iGPU, lGPU, tGPU, numNode, numLeafBody, numTriArray);



	//不要不行但要有何用？
	//估计是计算颜色值的时候要用这些初始值
	myCamera = new WCamera;
	//parameter的参数分别是 相机位置, 观察目标点, 上方, .......
	myCamera->setParameter(WVector3(Lx,Ly,Lz),/*cameraTarget*/WVector3(-1.0, -1.0, -1.0),WVector3(0,1,0),fov,winWidth*0.5/winHeight);
	myCamera->setFilmResolutionX(winWidth);
	myCamera->changeSampleSize(1);

	//添加材质

	//...没用的东西..
	/*WMaterial*perfectRefl=new WPerfectReflectionMaterial("PRL",3);
	((WPerfectReflectionMaterial*)perfectRefl)->setColor(WVector3(1));
	WMaterial*perfectRefr=new WPerfectRefractionMaterial("PRR",3,WVector3(1,1,0.8),1.5);
	WMaterial*metal=new WMetalMaterial("Metal",4,WVector3(0.8,0.8,0.8),3);
	WMaterial*phong=new WPhongMaterial("phong",3,WVector3(1,0.5,0),1,100);
	WMaterial*dielectric=new WDielectricMaterial("ddd",3,WVector3(1,0.5,0),100000,2.0);*/
//	myScene.setNthMaterial(metal,0);
//	myScene.setNthMaterial(dielectric,0);
//	myScene.setNthMaterial(perfectRefr,0);

	//添加灯光
	//只是一个点光源
	myLight=new WPointLight(WVector3(40), WVector3(1,-5,1));
	//myScene.addLight(myLight);
	myLight=new WPointLight(WVector3(50), WVector3(0,-2,3));
//	myScene.addLight(myLight);

	//对于宿舍的话，就是宿舍里面的灯，是某种矩形光源
	myLight=new WRectangleLight(WVector3(0,-5,8),WVector3(0,0,-1),WVector3(1,0,0),10,5,WVector3(25),true);
//	myScene.addLight(myLight);
	//对于宿舍的话，就是阳台的灯，也算是某种矩形光源
//	myLight=new WRectangleLight(WVector3(2,24,1),WVector3(0,-1,0),WVector3(0,0,1),4.5,6.5,WVector3(50),true);
//	myScene.addLight(myLight);

	//myIntegrator=new WPathIntegrator(&myScene,myAccelerator,3,WSampler::SAMPLER_RANDOM,1.0f);

//	myIntegrator = new WRecursivePathIntegrator(&myScene,myAccelerator,2,10);

/*	myIntegrator=new WIrradianceCacheIntegrator(
 		&myScene,myAccelerator,
  		20,50,20,10,3,5,
 			WSampler::SAMPLER_RANDOM,0.3,0.9,0.5,1.5);*/

}

void displayFcn()
{
	DWORD dwThreadId,dwThrdParam=1;
	HANDLE hThread;
	//显示模式设置
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glViewport(0,0,winWidth,winHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,winWidth,0,winHeight,-1,1);


	//将之前计算得到的颜色值绘制到屏幕上去
	myCamera->drawFilmInWorld(0,0,0);


	//用于绘制左下小方块
	/***********************左下角视窗绘制代码*******************************/
	
	
	if (isShowScene)
	{
		glEnable(GL_SCISSOR_TEST);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glScissor(0,0,winWidth/4,winHeight/4);
		glViewport(0,0,winWidth/4,winHeight/4);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(Lx,Ly,Lz,cameraTarget.x,cameraTarget.y,cameraTarget.z,0,0,1);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fovDeg,float(winWidth)/float(winHeight),
			0.1f,1000.0f);

		glColor3f(0.3,0.3,0.3);
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	//	glEnable(GL_CULL_FACE);
	//	glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);

		glBegin(GL_POINTS);
		glVertex3f(cameraTarget.x, cameraTarget.y, cameraTarget.z);
		glEnd();


		myCamera->drawCamera();
	//	myCamera->drawFilmInScreen();

	//	myFilm.draw();

	    //绘制左下小方框的内容物
		myScene.drawScene(false,false);
		/*
	//	myAccelerator->drawTree(0, 0,1,0);
	//	mySKD.drawTriangles();
	//	myTree.drawScene();
		myRay.tMin=1e-7f;
		myRay.tMax=M_INF_BIG;
		myRay.direction = WVector3(cos(raybeta)*cos(rayalpha),
								  cos(raybeta)*sin(rayalpha),sin(raybeta));
		myRay.drawSegment();
		glColor3f(0.7f,0.7f,0.7f);
		myRay2.drawSegment();

	#ifndef PACKET_INTERSECT
   		if(myAccelerator->intersect(myRay,DG))
  			DG.draw();
	#endif

		WBoundingBox box;
		box.pMin.x = -8.7500362;
		box.pMin.y = -1.3373880;
		box.pMin.z = 8.0343828;
		box.pMax.x = -8.7500353;
		box.pMax.y = -1.3051301;
		box.pMax.z = 8.0511990;
		glLineWidth(3.0f);
		glColor3f(1.0,0.0,0.0);
		box.draw();*/
		glColor3f(0.7f,0.7f,0.7f);
		glLineWidth(0.1f);

		const int n = 10;
		//以下的if语句块用于绘制左下小窗口的坐标轴
		if (isShowGrid)
		{
			//画出坐标轴
			glLineWidth(3.0f);
			glBegin(GL_LINES);
			glColor3f(1,0,0);
			glVertex3i(0,0,0);
			glVertex3i(n,0,0);
			glColor3f(0,1,0);
			glVertex3i(0,0,0);
			glVertex3i(0,n,0);
			glEnd();

			glLineWidth(0.5f);
			glBegin(GL_LINES);
			glColor3f(1,0,0);
			glVertex3i(0,0,0);
			glVertex3i(-n,0,0);
			glColor3f(0,1,0);
			glVertex3i(0,0,0);
			glVertex3i(0,-n,0);
			glEnd();

			glLineWidth(1.0f);
			glColor3f(0.7,0.7,0.7);
			glBegin(GL_LINES);
			for ( int i = -n; i <= n; ++i)
			{
				glVertex3i(i, -n, 0);
				glVertex3i(i, n, 0);
				glVertex3i(-n, i, 0);
				glVertex3i(n,i,  0);
			}
			glEnd();
		}



		if(!isComputing&&0)
		{
	//		((IrradianceCacheIntegrator*)myIntegrator)->displaySamplePoints();
	//		((IrradianceCacheIntegrator*)myIntegrator)->displayTreeNodes();
		}

		glDisable(GL_SCISSOR_TEST);
	}

	
	/***************************左下角视窗绘制代码End*********************************/
	

	/*******************************开启光线跟踪渲染的代码***********************************/
	//if语句块里面的东西要在glutMainLoop启动之后，运行一次，而且也只能运行一次
	if(!isInitiate){
		for (int ithThread = 0; ithThread < numCores; ++ithThread)
		{
			hThread=CreateThread(
				NULL,
				0,
				compute,
				(threadIDArray + ithThread),
				0,
				&dwThreadId);
			if(hThread==NULL)
			{
				//cout<<"创建线程失败"<<endl;
			}
		}
		glutPostRedisplay();
		isInitiate = !isInitiate;
	}else{
	}

	/***************************开启光线跟踪渲染计算代码End************************************/

	glutSwapBuffers();
	glFlush();


}
void mouseFcn(GLint button,GLint action,GLint xMouse,GLint yMouse)
{
	int mod=glutGetModifiers();
	if(mod==GLUT_ACTIVE_ALT)
		isAltKeyDown=true;
	else
		isAltKeyDown=false;
	if(button==GLUT_LEFT_BUTTON)
	{		
		if(action==GLUT_DOWN)
		{
			isLeftButtonDown=true;
			isFirstMove=true;
		}
		else if(action==GLUT_UP)
		{
			isLeftButtonDown=false;
			isFirstMove=true;
		}		
	}
	if(button==GLUT_MIDDLE_BUTTON)
	{
		if(action==GLUT_DOWN)
		{
			isMiddleButtonDown = true;
			isFirstMove = true;
		}
		else if(action==GLUT_UP)
		{
			isMiddleButtonDown = false;
			isFirstMove=true;
		}
	}
	if(button==GLUT_RIGHT_BUTTON)
	{
		if(action==GLUT_DOWN)
		{
			isRightButtonDown=true;
			isFirstMove=true;
		}
		else if(action==GLUT_UP)
		{
			isRightButtonDown=false;
			isFirstMove=true;
		}
	}
}

void mouseMoveFcn(GLint xMouse,GLint yMouse)
{
	float deltaX;
	float deltaY;
	if(isLeftButtonDown&&isAltKeyDown)
	{
		if(isFirstMove)
		{
			lastMouseX=xMouse;
			lastMouseY=yMouse;
			isFirstMove=false;
		}
		else
		{		
			deltaX=xMouse-lastMouseX;
			deltaY=yMouse-lastMouseY;
			lastMouseX=xMouse;
			lastMouseY=yMouse;
			alpha-=deltaX*0.004f;//deltaX;
			beta+=deltaY*0.004f;//deltaY;
			Lx=r*cos(beta)*cos(alpha) + cameraTarget.x;
			Ly=r*cos(beta)*sin(alpha) + cameraTarget.y;
			Lz=r*sin(beta) + cameraTarget.z;
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			gluLookAt(Lx,Ly,Lz,cameraTarget.x,cameraTarget.y,cameraTarget.z,0,0,1);
			myCamera->setParameter(WVector3(Lx,Ly,Lz),cameraTarget,WVector3(0,0,1),fov,(float)winWidth/(float)winHeight);
			glutPostRedisplay();
		}
		isEyeChanged = true;
	}
	if(isMiddleButtonDown && isAltKeyDown)
	{
		if(isFirstMove)
		{
			lastMouseX=xMouse;
			lastMouseY=yMouse;
			isFirstMove=false;
		}
		else
		{
			deltaX=xMouse-lastMouseX;
			deltaY=yMouse-lastMouseY;
			lastMouseX=xMouse;
			lastMouseY=yMouse;
			WVector3 x,y;
			myCamera->getXY(x, y);
			cameraTarget += -0.01f * x * deltaX + 0.01f * y * deltaY;
			Lx=r*cos(beta)*cos(alpha) + cameraTarget.x;
			Ly=r*cos(beta)*sin(alpha) + cameraTarget.y;
			Lz=r*sin(beta) + cameraTarget.z;
			myCamera->setParameter(WVector3(Lx,Ly,Lz),cameraTarget,WVector3(0,0,1),fov,(float)winWidth/(float)winHeight);
			glutPostRedisplay();
		}
		isEyeChanged = true;
	}
	if(isRightButtonDown&&isAltKeyDown)
	{
		if(isFirstMove)
		{
			lastMouseX=xMouse;
			lastMouseY=yMouse;
			isFirstMove=false;
		}
		else
		{
			deltaX=xMouse-lastMouseX;
			lastMouseX=xMouse;
			r-=deltaX*0.05f;
			Lx=r*cos(beta)*cos(alpha) + cameraTarget.x;
			Ly=r*cos(beta)*sin(alpha) + cameraTarget.y;
			Lz=r*sin(beta) + cameraTarget.z;
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			gluLookAt(Lx,Ly,Lz,cameraTarget.x,cameraTarget.y,cameraTarget.z,0,0,1);
			myCamera->setParameter(WVector3(Lx,Ly,Lz),cameraTarget,WVector3(0,0,1),fov,(float)winWidth/(float)winHeight);
			glutPostRedisplay();
		}
		isEyeChanged = true;

	}

}

void keyFcn(GLubyte key,GLint xMouse,GLint yMouse)
{
	DWORD dwThreadId,dwThrdParam=1;
	HANDLE hThread;
	switch (key)
	{
	case 'd':
		rayalpha+=0.015f;
		break;
	case 'a':
		rayalpha-=0.015f;
		break;
	case 's':
		raybeta+=0.015f;
		break;
	case 'w':
		raybeta-=0.015f;
		break;
	case 'r':
		if(!isComputing)
		{
			isComputing=true;

			for (int ithThread = 0; ithThread < numCores; ++ithThread)
			{
				hThread=CreateThread(
					NULL,
					0,
					compute,
					(threadIDArray + ithThread),
					0,
					&dwThreadId);
				if(hThread==NULL)
				{
					cout<<"创建线程失败"<<endl;
				}
				//Sleep(100);
			}
		}
		else
			isComputing = !isComputing;
		glutPostRedisplay();
		break;
	case 'g':
		isShowGrid = !isShowGrid;
		break;
	case 'e':
		isComputing = false;
		break;
	case 'v':
		isShowScene = !isShowScene;
	}

	glutPostRedisplay();

}
void reshapeFcn(GLint Width,GLint Height)
{
	//	cout<<"reshape"<<endl;
	winWidth=Width;
	winHeight=Height;

	glViewport(0,0,winWidth/4,winHeight/4);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovDeg,float(winWidth)/float(winHeight),
		0.1f,10000.0f);

	glViewport(0,0,winWidth,winHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,winWidth,0,winHeight,-1,1);

	if(!isComputing)
	{
		//渲染时禁止窗口缩放-->这里只是限定了窗口中画布的大小,窗口可以缩放但画布大小不变
		myCamera->setParameter(WVector3(Lx,Ly,Lz),cameraTarget,WVector3(0,0,1),fov,(float)winWidth/(float)winHeight);
		myCamera->setFilmResolutionX(winWidth);
	}
}


void timerFcn(int v)
{
	glutTimerFunc(500, timerFcn, -1);
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	

	glutInit(&argc,(char**)argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(winWidth,winHeight);
	glutCreateWindow("WRay");


	init();

//	glutIdleFunc(displayFcn);
	glutDisplayFunc(displayFcn);
	glutReshapeFunc(reshapeFcn);
	glutKeyboardFunc(keyFcn);
	glutMouseFunc(mouseFcn);
	glutMotionFunc(mouseMoveFcn);
	glutTimerFunc(500, timerFcn, -1);

	glutMainLoop();



	return 0;
}
