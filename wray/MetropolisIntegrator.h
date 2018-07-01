#pragma once

class WMetropolisIntegrator:SurfaceIntegrator
{
public:
	WMetropolisIntegrator(
		Scene*iscene,WAccelerator*itree,Camera* icamera);
	~WMetropolisIntegrator(void);
private:
	Camera* camera;
};
