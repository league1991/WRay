
#include "stdafx.h"

GLint winWidth=512,winHeight= 512;

float alpha=M_PI_2*3.0f,beta=0.0f;
float r=10;  //这个变量用于与alpha, beta协同确定Lx, Ly, Lz（也就是相机位置）的值
float fovDeg = 60;
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

Vector3 cameraTarget(0);
bool isEyeChanged = false;

void init()
{
	glClearColor(0.5,0.5,0.5,0.5);
	glEnableClientState(GL_VERTEX_ARRAY);

	//initialize camera
	Lx=r*cos(beta)*cos(alpha);
	Ly=r*cos(beta)*sin(alpha);
	Lz=r*sin(beta);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Lx,Ly,Lz,20,20,20,0,0,1);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovDeg,float(winWidth)/float(winHeight), 0.1f,1000.0f);

}

void displayFcn()
{
	//显示模式设置
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glViewport(0,0,winWidth,winHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,winWidth,0,winHeight,-1,1);

	//将之前计算得到的颜色值绘制到屏幕上去
	auto renderer = TileRenderer::getInstance();
	auto camera = renderer->getCamera();
	camera->drawFilmInWorld(0,0,0);

	//用于绘制左下小方块
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

		camera->drawCamera();

	    //绘制左下小方框的内容物
		renderer->getScene()->drawScene(false,false);
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
		glDisable(GL_SCISSOR_TEST);
	}

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
	auto camera = TileRenderer::getInstance()->getCamera();
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

			camera->setParameter(Vector3(Lx,Ly,Lz),cameraTarget,Vector3(0,0,1),fov,(float)winWidth/(float)winHeight);
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
			Vector3 x,y;
			camera->getXY(x, y);
			cameraTarget += -0.01f * x * deltaX + 0.01f * y * deltaY;
			Lx=r*cos(beta)*cos(alpha) + cameraTarget.x;
			Ly=r*cos(beta)*sin(alpha) + cameraTarget.y;
			Lz=r*sin(beta) + cameraTarget.z;
			camera->setParameter(Vector3(Lx,Ly,Lz),cameraTarget,Vector3(0,0,1),fov,(float)winWidth/(float)winHeight);
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
			camera->setParameter(Vector3(Lx,Ly,Lz),cameraTarget,Vector3(0,0,1),fov,(float)winWidth/(float)winHeight);
			glutPostRedisplay();
		}
		isEyeChanged = true;

	}

}

void keyFcn(GLubyte key,GLint xMouse,GLint yMouse)
{
	auto renderer = TileRenderer::getInstance();
	switch (key)
	{
	case 'r':
		if (!renderer->isRendering())
		{
			renderer->beginRender();
		}
		else
		{
			renderer->stopRender();
		}
		glutPostRedisplay();
		break;
	case 'g':
		isShowGrid = !isShowGrid;
		break;
	case 'e':
		break;
	case 'v':
		isShowScene = !isShowScene;
	}

	glutPostRedisplay();

}
void reshapeFcn(GLint Width,GLint Height)
{
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

	if(!TileRenderer::getInstance()->isRendering())
	{
		TileRenderer::getInstance()->setCamera(Vector3(Lx, Ly, Lz), cameraTarget, Vector3(0, 0, 1), fov, winWidth, winHeight);
	}
}


void timerFcn(int v)
{
	glutTimerFunc(1000, timerFcn, -1);
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		return 0;
	}

	glutInit(&argc,(char**)argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(winWidth,winHeight);
	glutCreateWindow("WRay");

	std::string fileName = argv[1];
	auto renderer = TileRenderer::getInstance();
	renderer->init();
	renderer->readScene(fileName);
	renderer->setCamera(Vector3(Lx, Ly, Lz), Vector3(-1.0, -1.0, -1.0), Vector3(0, 1, 0), fov, winWidth, winHeight);

	init();

	glutDisplayFunc(displayFcn);
	glutReshapeFunc(reshapeFcn);
	glutKeyboardFunc(keyFcn);
	glutMouseFunc(mouseFcn);
	glutMotionFunc(mouseMoveFcn);
	glutTimerFunc(1000, timerFcn, -1);

	glutMainLoop();
	return 0;
}
