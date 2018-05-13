#pragma once
#include "stdafx.h"
#include "Vector2.h"
#include "Vector3.h"
class DifferentialGeometry
{
public:
	Vector3 position;
	Vector3 normal,tangent,bitangent;
	Vector2f texCoord;
	Vector3 rayDir;
	Vector3 dpdu,dpdv;
	Vector3 dndu,dndv;
	unsigned int mtlId;

	DifferentialGeometry(void);//注意此函数没有进行初始化
	DifferentialGeometry(const DifferentialGeometry&DG);

	//此函数通过输入参数自动计算出主副切线
	DifferentialGeometry(
		const Vector3&iposition,const Vector3&inormal,
		const Vector3&irayDir,const Vector2f&itexCoord,
		const Vector3&idpdu,const Vector3&idpdv,
		const Vector3&idndu,const Vector2f&idndv,
		unsigned int imtlId);
	
	DifferentialGeometry(
		const Vector3&iposition,const Vector3&inormal,
		const Vector3&itangent,const Vector3&ibitangent,
		const Vector3&irayDir,const Vector2f&itexCoord,
		const Vector3&idpdu,const Vector3&idpdv,
		const Vector3&idndu,const Vector2f&idndv,
		unsigned int imtlId);
	virtual ~DifferentialGeometry(void);

	void draw();		//在openGL画出交点，调试时用
};
