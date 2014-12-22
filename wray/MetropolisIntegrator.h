#pragma once

class WMetropolisIntegrator:WSurfaceIntegrator
{
public:
	WMetropolisIntegrator(
		WScene*iscene,WAccelerator*itree,WCamera* icamera);
	~WMetropolisIntegrator(void);
private:
	WCamera* camera;
};
