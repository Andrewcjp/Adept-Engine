#include "GPUPerformanceTestManager.h"
#include "GPUPerformanceTest.h"
#include "Tests\PrimitveThoughputTest.h"
#include "Tests\GeometryProcessingTest.h"


GPUPerformanceTestManager::GPUPerformanceTestManager()
{}


GPUPerformanceTestManager::~GPUPerformanceTestManager()
{}

void GPUPerformanceTestManager::Init()
{
	if (!RHI::GetRenderSettings()->ShouldRunGPUTests)
	{
		return;
	}
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		TestSet[i].Device = RHI::GetDeviceContext(i);
		TestSet[i].Init();
	}
}

void GPUPerformanceTestManager::ExecuteAllTests()
{
	if (!RHI::GetRenderSettings()->ShouldRunGPUTests)
	{
		return;
	}
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		TestSet[i].ExecuteAllTests();
	}
}

void GPUPerformanceTestManager::GatherResults()
{
	if (!RHI::GetRenderSettings()->ShouldRunGPUTests)
	{
		return;
	}
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		TestSet[i].GatherResults();
	}
	LogResults();
}

void GPUPerformanceTestManager::AnalysizeResults()
{
	//Find Best set 
	//Check techniques
	//check connection amt

}

void GPUPerformanceTestManager::LogResults()
{
	Log::LogMessage("***Test Results***");
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		Log::LogMessage("GPU " + std::to_string(i) + " Results");
		TestSet[i].LogResults(&TestSet[0]);
	}
	Log::LogMessage("***End Test Results***");
}

void GPUPerformanceTestManager::DestoryTests()
{
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		TestSet[i].Destroy();
	}
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

void GPUPerformanceTestManager::GPUTestSet::GatherResults()
{
	for (int i = 0; i < EGPUPerformanceMetrics::Limit; i++)
	{
		if (Tests[i] != nullptr && Tests[i]->IsTestValid())
		{
			Tests[i]->GatherResults();
		}
	}
}

void GPUPerformanceTestManager::GPUTestSet::LogResults(GPUTestSet* ZeroSet)
{
	for (int i = 0; i < EGPUPerformanceMetrics::Limit; i++)
	{
		if (Tests[i] != nullptr && Tests[i]->IsTestValid())
		{
			Tests[i]->LogResults(ZeroSet->Tests[i]);
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
	Tests[EGPUPerformanceMetrics::GeometryProcessingSpeed] = new GeometryProcessingTest();
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
