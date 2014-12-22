#pragma once
#include "stdafx.h"
#include "Vector2.h"
#include "Vector3.h"
class WDifferentialGeometry
{
public:
	WVector3 position;
	WVector3 normal,tangent,bitangent;
	WVector2 texCoord;
	WVector3 rayDir;
	WVector3 dpdu,dpdv;
	WVector3 dndu,dndv;
	unsigned int mtlId;

	WDifferentialGeometry(void);//注意此函数没有进行初始化
	WDifferentialGeometry(const WDifferentialGeometry&DG);

	//此函数通过输入参数自动计算出主副切线
	WDifferentialGeometry(
		const WVector3&iposition,const WVector3&inormal,
		const WVector3&irayDir,const WVector2&itexCoord,
		const WVector3&idpdu,const WVector3&idpdv,
		const WVector3&idndu,const WVector2&idndv,
		unsigned int imtlId);
	
	WDifferentialGeometry(
		const WVector3&iposition,const WVector3&inormal,
		const WVector3&itangent,const WVector3&ibitangent,
		const WVector3&irayDir,const WVector2&itexCoord,
		const WVector3&idpdu,const WVector3&idpdv,
		const WVector3&idndu,const WVector2&idndv,
		unsigned int imtlId);
	virtual ~WDifferentialGeometry(void);

	void draw();		//在openGL画出交点，调试时用
};
