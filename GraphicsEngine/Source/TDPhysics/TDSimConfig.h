#pragma once
#include "TDTypes.h"

namespace TD
{
	class TDSimulationCallbacks;
	class TDSimConfig
	{
	public:
		TD_API TDSimConfig();
		//!Time Taken to sleep a body once still
		float BodySleepTime = 1.0f;
		//! Body values below this threshold will be zeroed off,Lower values will be more accurate but less performant in scenes with a large number of bodies
		float BodySleepZeroThreshold = 0.001f;
		//!Number of Iterations used to resolve collisions, higher is more accurate but require more CPU time
		int SolverIterationCount = 5;
		//! Number of Threads to use for the TaskGraph, 0 will auto select
		int TaskGraphThreadCount = 0;
		//! An instance of a Class to Handle any callbacks
		TDSimulationCallbacks* CallBackHandler = nullptr;
		//! The MaxDepth a Bounding Volume Hierarchy can be 
		int MaxBVHDepth = 7;

		TDBroadphaseMethod::Type BroadphaseMethod = TDBroadphaseMethod::SAP;
		typedef void(*FPrefCounterCallBack)(bool/*Is start of timer*/, TDPerfCounters::Type/*Timer ID*/);
		//! Callback Used for performance tracking in profile builds 
		FPrefCounterCallBack PerfCounterCallBack;
		typedef void(*FDebugLineCallBack)(glm::vec3 LineStart, glm::vec3 LineEnd, glm::vec3 Colour, float lifetime);
		FDebugLineCallBack DebugLineCallBack;

	};
};

