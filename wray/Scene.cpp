#include "StdAfx.h"
#include "Scene.h"
Scene::Scene(void)
{
	materials=NULL;
	nMaterials = nSubPrimitives = nTriangles = 0;
	nTriangles = NULL;
}

Scene::~Scene(void)
{
	clearScene();
}
void Scene::clearScene()
{
	m_objects.clear();
	for(unsigned int i=0;i<nMaterials;i++)
	{
		delete materials[i];
	}
	delete []materials;
	for(unsigned int i=0;i<m_lights.size();i++)
	{
		delete m_lights[i];
	}
	m_lights.clear();
	nSubPrimitives=0;
	nMaterials=0;
	clearTriangleArray();
}
void Scene::buildScene(ObjReader &reader)
{
	m_objects.resize(reader.primitives.size());
	unsigned int n=0;//每个Primitive的SubPrimitive数量
	for(unsigned int i=0;i<m_objects.size();i++)
	{
		reader.fillPrimitive(i,m_objects[i]);
		m_objects[i].getSubPrimNum(n);
		nSubPrimitives+=n;
	}
	buildSceneBBox();
	//创建整个场景的包围盒
	//创建材质数组
	nMaterials=reader.Materials.size();
	materials=new WMaterial*[nMaterials];
	string mtlName;
	float3 diffuse;
	float3 emission;
	for(unsigned int i=0;i<nMaterials;i++)
	{
		ObjMaterial& mtl = reader.Materials[i];
		mtlName=reader.Materials[i].name;
		diffuse=reader.Materials[i].diffuse;
		emission=reader.Materials[i].emission;
		//		cout<<mtlName<<diffuse.x<<diffuse.y<<diffuse.z<<endl;
		if (mtl.isShiny() && false)
		{ 
			materials[i] = new WPhongMaterial(mtlName, i, 
				Vector3(diffuse.x,diffuse.y,diffuse.z), 
				Vector3(mtl.specular.x, mtl.specular.y, mtl.specular.z),
				mtl.glossiness, Vector3(emission.x,emission.y,emission.z));
		}
		else if (mtl.isTransparent())
		{
			materials[i] = new WPerfectRefractionMaterial(mtlName, i, Vector3(diffuse.x,diffuse.y,diffuse.z), 1.33, Vector3(emission.x,emission.y,emission.z));
		}
		else
			materials[i]=new WLambertMaterial(mtlName,i,Vector3(diffuse.x,diffuse.y,diffuse.z),Vector3(emission.x,emission.y,emission.z));
	}
	buildTriangleArray();
	buildLightData();
}

void Scene::drawScene(bool showNormal,bool fillMode)
{
	WTriangle tri;
	Vector3 color;
	for(unsigned int i=0;i<m_objects.size();i++)
	{
		m_objects[i].getTriangle(0,tri);
		color=materials[tri.mtlId]->getColor();
		glColor3f(color.x,color.y,color.z);
		m_objects[i].drawPrimitive(showNormal,fillMode);
	}
	for(unsigned int i=0;i<m_lights.size();i++)
		m_lights[i]->draw();

}
void Scene::getObjects(MeshObject *&iprimitives, unsigned int &nPrims)
{
		nPrims= m_objects.size();
		iprimitives=&this->m_objects[0];
}
void Scene::getObject(MeshObject*&iprimitives,unsigned int nthPrim)
{
	iprimitives=&(m_objects[nthPrim]);
}
void Scene::getMaterials(WMaterial**&imaterials,unsigned int&nMtl)
{
	nMtl=nMaterials;
	imaterials=this->materials;
}
void Scene::getNthMaterial(WMaterial*&imaterial,unsigned int nthMtl)
{
	imaterial=materials[nthMtl];
}
void Scene::setNthMaterial(WMaterial*imaterial,unsigned int nthMtl)
{
	if(nthMtl>=0&&nthMtl<nMaterials)
	{
		delete materials[nthMtl];
		materials[nthMtl]=imaterial;
	}
}
void Scene::rebuildAllSubPs(unsigned int inFacesPerSubP)
{
	if(!inFacesPerSubP)
		return;
	nSubPrimitives=0;
	unsigned int n;
	for(unsigned int i=0;i<m_objects.size();i++)
	{
		m_objects[i].rebuildSubP(inFacesPerSubP);
		m_objects[i].getSubPrimNum(n);
		nSubPrimitives+=n;
	}
	return;
}
void Scene::buildSceneBBox()
{
	if(m_objects.size() ==0)
		return;
	sceneBox=m_objects[0].getBBox();
	for(unsigned int i=1;i<m_objects.size();i++)
	{
		//cout<<"mergebox"<<endl;
		sceneBox.merge(m_objects[i].getBBox());
	}
	return;
}
void Scene::buildLightData()
{
	std::unordered_map<WMaterial*, ObjectLight*> lightMap;

	for (int ithObj = 0; ithObj < m_objects.size(); ithObj++)
	{
		auto& object = m_objects[ithObj];
		WTriangle tri;
		unsigned int nTriangle;
		object.getFaceNum(nTriangle);
		for (int ithTri = 0; ithTri < nTriangle; ithTri++)
		{
			object.getTriangle(ithTri, tri);
			WMaterial* material = materials[tri.mtlId];
			if (material->isEmissive())
			{
				if (lightMap.find(material) == lightMap.end())
				{
					lightMap[material] = new ObjectLight(Vector3(1.0), tri.mtlId, this, true);
				}
				lightMap[material]->addTriangle(ithObj, ithTri);
			}
		}
	}

	for (auto& lightPair: lightMap)
	{
		m_lights.push_back(lightPair.second);
	}
}
void Scene::drawSceneBBox()
{
	glColor3f(0.1f,0.1f,0.5f);
	sceneBox.draw();
}
void Scene::addLight(Light *light)
{
	m_lights.push_back(light);
}
Light* Scene::getLightPointer(unsigned int nthLight)
{
	return m_lights[nthLight];
}
unsigned int Scene::getLightNum()
{
	return m_lights.size();
}
void Scene::clearSelect()
{
	for(unsigned int i=0;i<m_objects.size();i++)
		m_objects[i].isSelected=false;
}

void Scene::buildTriangleArray()
{
	unsigned int totalTris=0;
	for(unsigned int nthPrimitive=0;
		nthPrimitive<m_objects.size();nthPrimitive++)
	{
		unsigned int nFaces;
		m_objects[nthPrimitive].getFaceNum(nFaces);
		totalTris+=nFaces;
	}
	triangles=new WTriangle[totalTris];
//	nTriangles=totalTris;

	totalTris=0;
	for(unsigned int nthPrimitive=0;
		nthPrimitive<m_objects.size();nthPrimitive++)
	{
		unsigned int nFaces;
		m_objects[nthPrimitive].getFaceNum(nFaces);
		for(unsigned int nthFace=0;
			nthFace<nFaces; nthFace++)
		{
			WTriangle face;
			m_objects[nthPrimitive].getTriangle(nthFace,face);
			//忽略退化三角形
			if (face.point1 == face.point2 ||
				face.point2 == face.point3 ||
				face.point3 == face.point1)
			{
				continue;
			}
			triangles[totalTris]=face;
			triangles[totalTris].buildAccelerateData(totalTris);
			totalTris++;
		}
	}
	nTriangles = totalTris;
}

void Scene::drawByTriangleArray( bool showNormal/*=false*/,bool fillMode/*=false*/,Vector3 color/*=Vector3(0,0,0)*/ )
{
	glColor3f(color.x,color.y,color.z);
	for(unsigned int nthTriangle=0;
		nthTriangle<nTriangles;nthTriangle++)
	{
		triangles[nthTriangle].draw(showNormal,fillMode);
	}
}

void Scene::clearTriangleArray()
{
	if (!nTriangles)
	{
		return;
	}
	delete[] triangles;
	triangles=NULL;
	nTriangles = 0;
}

void Scene::getTriangleArray( WTriangle*&itriangles,unsigned int&nTris )
{
	itriangles=triangles;
	nTris=nTriangles;
}

void Scene::getTriAccelArray(float*& array, int& nFloat4s)
{
	nFloat4s = nTriangles * 3;
	array = new float[nFloat4s * 4];
	for (int ithTri = 0; ithTri < nTriangles; ++ithTri)
	{
		TriAccel& tA = triangles[ithTri].tA;
		float* begin = array + ithTri * 12;
		begin[0] = tA.nu;
		begin[1] = tA.nv;
		begin[2] = tA.np;
		begin[4] = tA.pu;
		begin[5] = tA.pv;
		begin[8] = tA.e0u;
		begin[9] = tA.e0v;
		begin[10] = tA.e1u;
		begin[11] = tA.e1v;
		int& u = *((int*)(begin + 3));
		int& v = *((int*)(begin + 6));
		int& w = *((int*)(begin + 7));
		char ci = tA.ci >> 25;
		w = ci & 0x3;			ci >>= 2;
		v = ci & 0x3;			ci >>= 2;
		u = ci & 0x3;
	}
}

void Scene::getMaterialArrayFloat4Uint2( 
										unsigned int*& mtlIDs, unsigned int& nIDPixels, 
										float*& mtlData, unsigned int& nDataPixels )
{
	//分配索引像素的空间
	//材质索引像素为uint2类型
	nIDPixels = nMaterials;
	mtlIDs = new unsigned int[nIDPixels * 2];
	vector<float> dataVector, temp;
	unsigned int mtlAddress = 0;
	for (unsigned int ithMtl = 0; ithMtl < nMaterials; ithMtl++)
	{
		//mtlIDs的一条记录，
		//第一个是材质的类型
		//第二个是材质的起始地址,
		//由于材质属性纹理为float4类型的
		//所以需要除以4，也就是向右移2位
		mtlIDs[(ithMtl<<1)] = materials[ithMtl]->getType();
		mtlIDs[(ithMtl<<1) + 1] = dataVector.size()>>2;
		//把材质数据导到一个暂存向量中
		materials[ithMtl]->getProperties(temp);
		dataVector.insert(dataVector.end(), temp.begin(), temp.end());
	}
	//从暂存的向量把数据正式输出
	nDataPixels = dataVector.size()>>2;
	mtlData = new float[dataVector.size()];
	for (unsigned int ithElement = 0; ithElement < dataVector.size(); 
		ithElement++)
	{
		mtlData[ithElement] = dataVector[ithElement];
	}
}

void Scene::getLightArrayFloat4Uint2( 
									 unsigned int*& lightIDs, unsigned int& nIDPixels, 
									 float*& lightData, unsigned int& nDataPixels )
{
	//分配索引像素的空间
	//材质索引像素为uint2类型
	nIDPixels = m_lights.size();
	lightIDs = new unsigned int[nIDPixels * 2];
	vector<float> dataVector, temp;
	unsigned int lightAddress = 0;
	for (unsigned int ithLight = 0; ithLight < m_lights.size(); ithLight++)
	{
		//lightIDs的一条记录，
		//第一个是材质的类型
		//第二个是材质的起始地址,
		//由于材质属性纹理为float4类型的
		//所以需要除以4，也就是向右移2位
		lightIDs[ithLight<<1] = m_lights[ithLight]->type;
		lightIDs[(ithLight<<1) + 1] = dataVector.size()>>2;
		//把材质数据导到一个暂存向量中
		m_lights[ithLight]->getProperties(temp);
		dataVector.insert(dataVector.end(), temp.begin(), temp.end());
	}
	//从暂存的向量把数据正式输出
	nDataPixels = dataVector.size()>>2;
	lightData = new float[dataVector.size()];
	for (unsigned int ithElement = 0; ithElement < dataVector.size(); 
		ithElement++)
	{
		lightData[ithElement] = dataVector[ithElement];
	}
}