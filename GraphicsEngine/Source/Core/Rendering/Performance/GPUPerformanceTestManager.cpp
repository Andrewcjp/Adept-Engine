#include "GPUPerformanceTestManager.h"
#include "GPUPerformanceTest.h"
#include "Tests\PrimitveThoughputTest.h"


GPUPerformanceTestManager::GPUPerformanceTestManager()
{}


GPUPerformanceTestManager::~GPUPerformanceTestManager()
{}

void GPUPerformanceTestManager::Init()
{
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		TestSet[i].Device = RHI::GetDeviceContext(i);
		TestSet[i].Init();
	}
}

void GPUPerformanceTestManager::ExecuteAllTests()
{
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		TestSet[i].ExecuteAllTests();
	}
}

void GPUPerformanceTestManager::DestoryTests()
{

}

void GPUPerformanceTestManager::GPUTestSet::ExecuteAllTests()
{
	for (int i = 0; i < EGPUPerformanceMetrics::Limit; i++)
	{
		if (Tests[i] != nullptr && Tests[i]->IsTestValid())
		{
			Tests[i]->RunTest();
		}
	}
}

void GPUPerformanceTestManager::GPUTestSet::Init()
{
	if (Device == nullptr)
	{
		return;
	}
	Tests[EGPUPerformanceMetrics::PrimitveThoughput] = new PrimitveThoughputTest();
	for (int i = 0; i < EGPUPerformanceMetrics::Limit; i++)
	{
		if (Tests[i] != nullptr && Tests[i]->IsTestValid())
		{
			Tests[i]->Init(Device);
		}
	}
}

void GPUPerformanceTestManager::GPUTestSet::Destroy()
{
	for (int i = 0; i < EGPUPerformanceMetrics::Limit; i++)
	{
		if (Tests[i] != nullptr)
		{
			Tests[i]->Destroy();
		}
	}
}
