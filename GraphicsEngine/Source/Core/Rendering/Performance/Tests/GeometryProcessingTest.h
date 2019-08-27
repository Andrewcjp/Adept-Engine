#pragma once
#include "..\GPUPerformanceTest.h"
class GeometryProcessingTest : public GPUPerformanceTest
{
public:
	GeometryProcessingTest();
	~GeometryProcessingTest();

	virtual void RunTest() override;
	virtual void GatherResults() override;
	virtual void LogResults(GPUPerformanceTest* ZeroTest) override;
protected:
	virtual void OnInit() override;

};

