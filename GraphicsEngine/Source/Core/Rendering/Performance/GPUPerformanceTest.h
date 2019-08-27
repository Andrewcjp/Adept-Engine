#pragma once
#include "GPUPerformanceTestManager.h"
//A single test
//Must be run off screen
//Cannot change the current renderer state.
//Cannot tick the GPU.
//on screen is optional and is handled in a special node.
class GPUPerformanceTest
{
public:
	GPUPerformanceTest();
	virtual ~GPUPerformanceTest();

	virtual bool IsTestValid();
	virtual void RunTest();
	void Init(DeviceContext* Context);
	void Destroy();
	TestResultsData GetResults();
	//populate the ResultsData struct 
	virtual void GatherResults();
	virtual void LogResults(GPUPerformanceTest* ZeroTest);
	TestResultsData ResultData;
protected:
	virtual void OnInit();
	virtual void OnDestory();
	DeviceContext* Device = nullptr;
	
};

