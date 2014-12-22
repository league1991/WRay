#include "stdafx.h"
void WDisplaySystem::drawWireArray(WGLWireArray&vb)
{
	glVertexPointer(3,GL_FLOAT,0,vb.vertices);
	glDrawElements(GL_TRIANGLES,vb.nFaces*3,GL_UNSIGNED_INT,vb.indices);
}