#pragma once
#include "time.h"
class WClock
{
public:
	WClock(void){totalTime=0;}
	~WClock(void);

	void begin();
	void end();
	void reset(){totalTime=0;}
	void display()
	{cout<<(float)totalTime/(float)CLOCKS_PER_SEC<<" s"<<endl;};
private:
	clock_t beginTime;
	clock_t totalTime;
};
