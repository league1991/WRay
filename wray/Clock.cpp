#include "StdAfx.h"
#include "Clock.h"
#include <time.h>


WClock::~WClock(void)
{
}

void WClock::begin()
{
	beginTime=clock();
}
void WClock::end()
{
	totalTime+=clock()-beginTime;
}