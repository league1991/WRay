#include "stdafx.h"
#include "Primitive.h"
void WSubPrimitive::draw(bool isFill)
{
	for(unsigned int nthFace=beginIndex;
		nthFace<nFaces+beginIndex;
		nthFace++)
	{
		WTriangle tri;
		pPrimitive->getTriangle(nthFace,tri);
		tri.draw(false,isFill);
	}
}
WPrimitive::WPrimitive(void)
{
	this->nFacesPerSubP=50;
	isSelected=false;
}

WPrimitive::~WPrimitive(void)
{
	this->clear();
}
bool WPrimitive::getTriangle(unsigned int nthFace,WTriangle&tri)
{
	if(nthFace>=nFaces||nthFace<0)
		return false;
	float*p;
	//找到顶点位置
	p=&vertices[vertexIndices[nthFace*3]*3];
	tri.point1=WVector3(*p,*(p+1),*(p+2));
	p=&vertices[vertexIndices[nthFace*3+1]*3];
	tri.point2=WVector3(*p,*(p+1),*(p+2));
	p=&vertices[vertexIndices[nthFace*3+2]*3];
	tri.point3=WVector3(*p,*(p+1),*(p+2));

	//找到法向量
	p=&normals[normalIndices[nthFace*3]*3];
	tri.normal1=WVector3(*p,*(p+1),*(p+2));
	p=&normals[normalIndices[nthFace*3+1]*3];
	tri.normal2=WVector3(*p,*(p+1),*(p+2));
	p=&normals[normalIndices[nthFace*3+2]*3];
	tri.normal3=WVector3(*p,*(p+1),*(p+2));

	//找到贴图坐标
	p=&texcoords[texCoordIndices[nthFace*3]*2];
	tri.texCoord1=WVector2(*p,*(p+1));
	p=&texcoords[texCoordIndices[nthFace*3+1]*2];
	tri.texCoord2=WVector2(*p,*(p+1));
	p=&texcoords[texCoordIndices[nthFace*3+2]*2];
	tri.texCoord3=WVector2(*p,*(p+1));

	tri.mtlId=mtlIndices[nthFace];
	return true;
}
void WPrimitive::buildBBox()
{
	if(nVertices<1)
		return;
	WVector3 delta=WVector3(WBoundingBox::delta);
	box.pMin=WVector3(vertices[0],vertices[1],vertices[2])-delta;
	box.pMax=WVector3(vertices[0],vertices[1],vertices[2])+delta;
	for(unsigned int i=1;i<nVertices;i++)
	{
		box.merge(
			WVector3(vertices[3*i],vertices[3*i+1],vertices[3*i+2])
			);
	}
	return;
}
void WPrimitive::drawBBox()
{
	box.draw();
}
void WPrimitive::drawPrimitive(bool showNormal,bool fillMode)
{
	WTriangle t;
	for(unsigned int i=0;i<nFaces;i++)
	{
		getTriangle(i,t);
		t.draw(showNormal,fillMode);
	}
}
void WPrimitive::clear()
{
	delete[]vertices;
	delete[]normals;
	delete[]texcoords;
	delete[]vertexIndices;
	delete[]normalIndices;
	delete[]texCoordIndices;
	delete[]mtlIndices;
	delete[]subPs;
	vertices = normals = texcoords = NULL;
	vertexIndices = normalIndices = texCoordIndices = mtlIndices = NULL;
	subPs = NULL;
	nVertices=nTexcoords=nNormals=nFaces=nSubPs=0;
}
void WPrimitive::buildSubP()
{
	if(nFaces==0)
		return;
	nSubPs=(nFaces+nFacesPerSubP-1)/nFacesPerSubP;
	//cout<<nFaces<<' '<<nSubPs<<endl;

	//最后一个SubPrimitive包含的面数
	unsigned int lastNFace=nFaces-nFacesPerSubP*(nSubPs-1);
	subPs=new WSubPrimitive[nSubPs];
	int nthSubP=-1;
	WTriangle t;
	for(unsigned int i=0;i<nFaces;i++)
	{
		getTriangle(i,t);//获得第i个三角形
		if(i%nFacesPerSubP==0)//获得下一个subp
		{
			WVector3 delta=WVector3(WBoundingBox::delta);
			nthSubP++;
			subPs[nthSubP].box.pMin=t.point1-delta;
			subPs[nthSubP].box.pMax=t.point1+delta;
			subPs[nthSubP].beginIndex=i;
			subPs[nthSubP].nFaces=nFacesPerSubP;
			subPs[nthSubP].pPrimitive=this;
		}
		subPs[nthSubP].box.merge(t);
	}
	subPs[nthSubP].nFaces=lastNFace;
}
void WPrimitive::drawSubPBBox()
{
	glColor3f(0.2f,0.2f,0.2f);
	for(unsigned int i=0;i<nSubPs;i++)
		subPs[i].box.draw();
	return;
}
void WPrimitive::rebuildSubP(int inFacesPerSubP)
{
	if(inFacesPerSubP<=0)
		return;
	nFacesPerSubP=inFacesPerSubP;
	delete[]subPs;
	buildSubP();
}
void WPrimitive::getSubPrimitives(WSubPrimitive*&isubPs,unsigned int&nSubP)
{
		isubPs=this->subPs;
		nSubP=nSubPs;
}
