#include "stdafx.h"
#include <stdio.h>
#include "renderKernel.h"

extern "C"
{
	void wray_kernel(Render* re)
	{
		RenderResult* result = re->result;
		int width = result->rectx;
		int height = result->recty;

		WScene scene;
		scene.buildScene(re);

		WSimpleKD tree;
		tree.buildTree(scene);
		tree.displayNodeInfo();

		WCamera camera;
		WVector3 target	(0.0f, 0.0f, -1.0f);
		WVector3 origin	(0.0f, 0.0f, 0.0f);
		WVector3 up		(0.0f, 1.0f, 0.0f);
		Camera* blCamera = (Camera*)re->scene->camera->data;
		mul_m4_v3(re->scene->camera->obmat, origin.v);
		mul_m4_v3(re->scene->camera->obmat, target.v);
		mul_m4_v3(re->scene->camera->obmat, up.v);
		up = up - origin;
		float ratio = float(width) / float(height);
		float theta = 2.0f * atan(16.0f / ratio / blCamera->lens);
		camera.setParameter(origin, target, up, theta, ratio);
		camera.setFilmResolutionXY(width, height);
		camera.changeSampleSize(1);

		WLight* light = new WPointLight(WVector3(20),WVector3(0,0,5));
		scene.addLight(light);

		WPathIntegrator integrator(&scene,&tree,2,WSampler::SAMPLER_RANDOM,1.0f);

		typedef struct Pixel
		{
			float r,g,b,a;
		}Pixel;
		RenderLayer* layer = (RenderLayer*)result->layers.first;
		Pixel* pPixel = (Pixel*)layer->rectf;
		int iteration = 1;
		while (iteration)
		{
			WRay ray;
			WVector3 color;
			if (camera.isFilmFull())
			{
				printf("%d th iteration\n", iteration);
				pPixel = (Pixel*)layer->rectf;
				for(int j = 0; j < height; ++j)
				{
					for(int i = 0; i < width; ++i)
					{
						color = camera.getColor(i, j);
						pPixel->r = color.x;
						pPixel->g = color.y;
						pPixel->b = color.z;
						pPixel->a = 1.0f;
						pPixel++;
					}
				}
				camera.nextExposurePass();
				re->display_draw(re->ddh, result, NULL);
				iteration++;
			}
			else
			{
				camera.getNextRay(ray);
				color = integrator.integrate(ray);
				camera.setColor(color.x,color.y,color.z);
			}
			if (re->test_break(re->tbh))
				iteration = 0;
		}
		tree.displayIsectStatistics();
	}
}