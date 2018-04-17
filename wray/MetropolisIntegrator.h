#pragma once

class WMetropolisIntegrator:WSurfaceIntegrator
{
public:
	WMetropolisIntegrator(
		Scene*iscene,WAccelerator*itree,Camera* icamera);
	~WMetropolisIntegrator(void);
private:
	Camera* camera;
};
