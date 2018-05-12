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
#include "GL/freeglut.h"
#include <time.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <deque>
#include <math.h>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>
#include <omp.h>
#include <xmmintrin.h>
#include <emmintrin.h>
using namespace std;

#include <math.h>
#include <string.h>
#include <stddef.h>

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
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
#include "TileRenderer.h"


#define  M_PI   3.1415926535    
#define  M_2_PI M_PI*2.0
#define  M_PI_2 M_PI*0.5
#define  maxf(x,y) ((x>y)?x:y)
#define  minf(x,y) ((x<y)?x:y)