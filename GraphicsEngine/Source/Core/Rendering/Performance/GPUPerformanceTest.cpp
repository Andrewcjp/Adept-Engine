#include "GPUPerformanceTest.h"


GPUPerformanceTest::GPUPerformanceTest()
{}


GPUPerformanceTest::~GPUPerformanceTest()
{}

bool GPUPerformanceTest::IsTestValid()
{
	return true;
}

void GPUPerformanceTest::RunTest()
{

}

void GPUPerformanceTest::Init(DeviceContext * Context)
{
	Device = Context;
	OnInit();
}

void GPUPerformanceTest::Destroy()
{
	OnDestory();
}

TestResultsData GPUPerformanceTest::GetResults()
{
	return ResultData;
}

void GPUPerformanceTest::GatherResults()
{}

void GPUPerformanceTest::OnInit()
{}

void GPUPerformanceTest::OnDestory()
{}
