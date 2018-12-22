
#include "TDSimConfig.h"
#include <thread>
#include <algorithm>

TD::TDSimConfig::TDSimConfig()
{
	BodySleepZeroThreshold = 0.001f;
	SolverIterationCount = 5;
	int cpucount = std::thread::hardware_concurrency();
	TaskGraphThreadCount = std::max((int)1, cpucount - 2);
	BroadphaseMethod = TDBroadphaseMethod::SAP;
}
