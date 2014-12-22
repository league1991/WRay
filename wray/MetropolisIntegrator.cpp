#include "StdAfx.h"
#include "MetropolisIntegrator.h"

WMetropolisIntegrator::WMetropolisIntegrator(	
	WScene*iscene,WAccelerator*itree,WCamera* icamera):WSurfaceIntegrator(iscene,itree)
{
	tree = itree;
	scene = iscene;
	camera = icamera;
}

WMetropolisIntegrator::~WMetropolisIntegrator(void)
{
}
