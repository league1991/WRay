#pragma once
#include "stdafx.h"

struct WGLWireArray
{
	float*vertices;
	unsigned*indices;
	int nFaces;
};
class WDisplaySystem
{
public:
	void drawWireArray(WGLWireArray &vb);
};