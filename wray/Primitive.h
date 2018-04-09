#pragma once
//#include "stdafx.h"
#include <gl/glut.h>
#include "Triangle.h"
#include "BoundingBox.h"
class MeshObject;
struct WSubPrimitive
{
	unsigned int beginIndex;
	unsigned int nFaces;
	WBoundingBox box;
	MeshObject*pPrimitive;//到所在Primitive的指针
	void draw(bool isFill);//画出包含的三角形
};
class MeshObject
{
	//ObjReader是Primitive的友元类，可以直接访问它的私有成员
	friend class WObjReader;
	friend class WScene;
public:
	MeshObject(void);
	virtual ~MeshObject(void);

	bool isSelected;			//表示其是否被选中

	//获得三角形
	bool getTriangle(unsigned int nthFace,WTriangle&tri);

	void drawBBox();			//画出整个Primitive的包围盒
	bool isIntersectWBBox(WRay&ray)//测试与包围盒是否相交
	{float tmin,tmax;return box.isIntersect(ray,tmin,tmax);}

	//画出整个Primitive,调试时用
	void drawPrimitive(bool showNormal=false,bool fillMode=false);
	void clear();				//清理存储空间
	void drawSubPBBox();		//画出SubPrimitive的包围盒
	
	//重建SubPrimitive，输入每个SubPrimitive的面数
	void rebuildSubP(int inFacesPerSubP);
	//获得每个SubPrimitive的指针，以及元素个数
	void getSubPrimitives(WSubPrimitive*&isubPs,unsigned int&nSubP);
	//获得总的subPrimitive的个数
	void getSubPrimNum(unsigned int&insubP)
	{insubP=nSubPs;}
	void getFaceNum(unsigned int &inFace)
	{inFace=nFaces;}
	//获得包围盒
	WBoundingBox getBBox(){return this->box;}
private:
	unsigned int nVertices;		//总的顶点数=顶点数组大小/3
	unsigned int nTexcoords;	//总的贴图坐标数=贴图坐标数组大小/2
	unsigned int nNormals;		//总的法向量数=法向量数组大小/3
	unsigned int nFaces;		//总的面数=索引数/3
	unsigned int nSubPs;		//总子图元(SubPrimitive)数
	unsigned int nFacesPerSubP;	//每个子图元的面数

	float*vertices;				//顶点数组
	float*normals;				//法向量数组
	float*texcoords;			//贴图坐标数组

	unsigned int*vertexIndices;	//顶点索引数组
	unsigned int*normalIndices;	//法向量索引数组
	unsigned int*texCoordIndices;//贴图坐标索引数组
	unsigned int*mtlIndices;	//材质索引数组

	WSubPrimitive *subPs;
	WBoundingBox box;

	void buildBBox();			//创建整个Primitive的包围盒
	void buildSubP();			//构建子图元，此函数在ObjReader类被调用
};
