#pragma once

#include "ObjReader.h"
#include "Primitive.h"
#include "Material.h"
#include "Light.h"
class Scene
{
public:
	Scene(void);
	~Scene(void);

	//此函数把ObjReader得到的数据导入Scene类之中
	void buildScene(ObjReader&reader);
	//清除场景
	void clearScene();
	//此函数画出整个场景
	void drawScene(bool showNormal=false,bool fillMode=false);
	//此函数画出整个场景，利用TriangleArray作为数据
	//可以此检查TriangleArray是否有错
	void drawByTriangleArray(bool showNormal=false,bool fillMode=false,Vector3 color=Vector3(0,0,0));
	//此函数重建所有物体的subPrimitive,参数表示每个SubPrimitive包含的最大面数,调用此函数后必须重建KD树
	void rebuildAllSubPs(unsigned int inFacesPerSubP);
	//获得基本体的指针，以及基本体的个数
	void getObjects(MeshObject*&iprimitives,unsigned int&nPrims);
	void getObject(MeshObject*&iprimitives,unsigned int nthPrim);
	//获得材质指针数组的指针，以及材质的个数
	void getMaterials(Material**&imaterials,unsigned int&nMtl);
	//获得第nthMtl个材质
	void getNthMaterial(Material*&imaterial,unsigned int nthMtl);
	//设置第nthMtl个材质
	void setNthMaterial(Material*imaterial,unsigned int nthMtl);
	//返回基本体的总数
	unsigned int getPrimNum(){return m_objects.size();}
	//返回子基本体的总数
	unsigned int getSubPrimNum(){return nSubPrimitives;}
	//返回场景的包围盒
	WBoundingBox getBBox(){return sceneBox;}
	//画出整个场景的包围盒,颜色为青绿色
	void drawSceneBBox();
	//往场景添加灯光
	void addLight(Light *light);
	Light* getLightPointer(unsigned int nthLight);
	unsigned int getLightNum();
	//表示不选择所有物体
	void clearSelect();
	void getTriangleArray(WTriangle*&itriangles,unsigned int&nTris);
	void getTriAccelArray(float*& array, int& nElements);

	//以下函数把材质和灯光数据返回到对应的数组里面
	//用于做成纹理给GPU使用
	//1）材质方面，包含一个索引数组和一个数据数组
	//   索引数组存储材质的类型和起始位置（就是在数据纹理的第几个像素）
	//   数据数组存储各种材质的属性，例如表面颜色
	//2）灯光方面，包含一个索引数组和一个数据数组
	//   索引数组存储灯光的类型和起始位置（就是在数据纹理的第几个像素）
	//   数据数组存储各种灯光的属性，例如强度，位置
	void getMaterialArrayFloat4Uint2(
		unsigned int*& mtlIDs,  unsigned int& nIDPixels,
		float*& mtlData,		unsigned int& nDataPixels);
	//   数据数组存储各种灯光的属性，例如强度，位置
	void getLightArrayFloat4Uint2(
		unsigned int*& lightIDs,  unsigned int& nIDPixels,
		float*& lightData,		unsigned int& nDataPixels);

	//for testing
	void showTriangles(){
		for(int i = 0; i < this->nTriangles; i ++){
			printf("tri%d p1: %f, %f, %f;\n", i, this->triangles[i].point1.x, this->triangles[i].point1.y, this->triangles[i].point1.z);
			printf("tri%d p2: %f, %f, %f;\n", i, this->triangles[i].point2.x, this->triangles[i].point2.y, this->triangles[i].point2.z);
			printf("tri%d p3: %f, %f, %f;\n", i, this->triangles[i].point3.x, this->triangles[i].point3.y, this->triangles[i].point3.z);
		}
	}

	void showMaterials(){
		for(int i = 0; i < this->nMaterials; i ++){
			Vector3 tmp = this->materials[i]->getColor();
			printf("mtl%d: %f, %f, %f;\n", i, tmp.x, tmp.y, tmp.z);
		}
	}

    void setEnvironmentLight(Light* light);
    Light* getEnvironmentLight() { return m_envLight; }

private:
	vector<Light*>m_lights;					//存储灯光指针
    Light* m_envLight;                      // Environment Light
	Material**materials;					//Material指针数组
	unsigned int nMaterials;				//Material总数
	std::vector<MeshObject>m_objects;		//Primitive数组
	unsigned int nSubPrimitives;			//SubPrimitive总数
	WBoundingBox sceneBox;					//整个场景的包围盒

	WTriangle*triangles;					//三角形数组
	unsigned int nTriangles;				//三角形个数
	void buildTriangleArray();				//创建三角形数组
	void clearTriangleArray();				//清除三角形数组

	void buildSceneBBox();					//创建场景包围盒
	void buildLightData();					// Build light data
};
