#ifndef RENDERKERNEL_H
#define RENDERKERNEL_H
struct Render;
struct RenderJob;
//#include "stdafx.h"
#ifdef __cplusplus
extern "C" {
#endif

	void wray_kernel(Render* re);

#ifdef __cplusplus
}
#endif

#endif