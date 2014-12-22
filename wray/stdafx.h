// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

// TODO: 在此处引用程序需要的其他头文件
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#define GLUT_DISABLE_ATEXIT_HACK //这句话极其重要
#include <GL/glut.h>
#include <time.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <deque>
#include <math.h>
#include <string>
#include <algorithm>
#include <omp.h>
#include <xmmintrin.h>
#include <emmintrin.h>
using namespace std;

#include <math.h>
#include <string.h>
#include <stddef.h>

//#define BLENDER_INCLUDE

//#include "BLI_math.h"
#ifdef BLENDER_INCLUDE

#include "MEM_guardedalloc.h"

#include "BLI_blenlib.h"
#include "BLI_math_matrix.h"
#include "BLI_threads.h"
#include "BLI_rand.h"
#include "BLI_utildefines.h"
//#include "BLI_callbacks.h"
#include "BLI_memarena.h"

#include "PIL_time.h"

#include "DNA_group_types.h"
#include "DNA_image_types.h"
#include "DNA_node_types.h"
#include "DNA_object_types.h"
#include "DNA_scene_types.h"
#include "DNA_sequence_types.h"
#include "DNA_userdef_types.h"
#include "DNA_meshdata_types.h"
#include "DNA_material_types.h"
#include "DNA_camera_types.h"

#include "BKE_blender.h"
#include "BKE_context.h"
#include "BKE_global.h"
#include "BKE_image.h"
#include "BKE_library.h"
#include "BKE_main.h"
#include "BKE_node.h"
#include "BKE_multires.h"
#include "BKE_material.h"
#include "BKE_report.h"
#include "BKE_sequencer.h"
//#include "BKE_screen.h"
#include "BKE_scene.h"
#include "BKE_DerivedMesh.h"

#include "WM_api.h"
//#include "WM_types.h"

#include "ED_screen.h"
//#include "ED_object.h"

#include "RE_pipeline.h"

#include "IMB_imbuf.h"
#include "IMB_imbuf_types.h"

#include "RNA_access.h"
#include "RNA_define.h"

#include "wm_window.h"

#include "render_types.h"
//#include "render_intern.h"
#include "renderpipeline.h"
//#include "render_internal.c"
#include "renderdatabase.h"
#include "texture.h"
#endif


#include "Vector2.h"
#include "Vector3.h"
#include "DisplaySystem.h"
#include "ObjReader.h"
#include "Ray.h"
#include "Primitive.h"
#include "DifferentialGeometry.h"
#include "Triangle.h"
#include "BoundingBox.h"
#include "Clipper.h"
#include "Scene.h"
#include "Accelerator.h"
#include "SimpleBVH.h"
#include "MultiBVH.h"
//#include "MultiRSBVH.h"
#include "SimpleKD.h"
#include "SimpleKD2.h"
//#include "EnhancedKD.h"
//#include "DualKD.h"
#include "Camera.h"
#include "Film.h"
#include "Sample.h"
#include "Sampler.h"
#include "Filter.h"
#include "MC.h"
#include "BSDF.h"
#include "Light.h"
#include "SurfaceIntegrator.h"
#include "DirectLighting.h"
#include "Clock.h"
#include "pathIntegrator.h"
#include "RecursivePathIntegrator.h"
#include "OctTree.h"
#include "IrradianceCacheIntegrator.h"
#include "RandomCamera.h"
#include "MetropolisIntegrator.h"
#include "renderKernel.h"


#define  M_PI   3.1415926535    
#define  M_2_PI M_PI*2.0
#define  M_PI_2 M_PI*0.5
#define  maxf(x,y) ((x>y)?x:y)
#define  minf(x,y) ((x<y)?x:y)