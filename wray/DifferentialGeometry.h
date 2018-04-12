#pragma once
#include "stdafx.h"
#include "Vector2.h"
#include "Vector3.h"
class WDifferentialGeometry
{
public:
	Vector3 position;
	Vector3 normal,tangent,bitangent;
	Vector2 texCoord;
	Vector3 rayDir;
	Vector3 dpdu,dpdv;
	Vector3 dndu,dndv;
	unsigned int mtlId;

	WDifferentialGeometry(void);//注意此函数没有进行初始化
	WDifferentialGeometry(const WDifferentialGeometry&DG);

	//此函数通过输入参数自动计算出主副切线
	WDifferentialGeometry(
		const Vector3&iposition,const Vector3&inormal,
		const Vector3&irayDir,const Vector2&itexCoord,
		const Vector3&idpdu,const Vector3&idpdv,
		const Vector3&idndu,const Vector2&idndv,
		unsigned int imtlId);
	
	WDifferentialGeometry(
		const Vector3&iposition,const Vector3&inormal,
		const Vector3&itangent,const Vector3&ibitangent,
		const Vector3&irayDir,const Vector2&itexCoord,
		const Vector3&idpdu,const Vector3&idpdv,
		const Vector3&idndu,const Vector2&idndv,
		unsigned int imtlId);
	virtual ~WDifferentialGeometry(void);

	void draw();		//在openGL画出交点，调试时用
};
