#pragma once
namespace TD
{
	class TDSimConfig
	{
	public:
		TDSimConfig();
		//!Time Taken to sleep a body once still
		float BodySleepTime = 1.0f;
		/**Body values below this threshold will be zeroed off,
		Lower values will be more accurate but less performant in scenes with a large number of bodies*/
		float BodySleepZeroThreshold = 0.001f;
		//!Number of Iterations used to resolve collisions, higher is more accurate but require more CPU time
		int SolverIterationCount = 5;
	};
};

