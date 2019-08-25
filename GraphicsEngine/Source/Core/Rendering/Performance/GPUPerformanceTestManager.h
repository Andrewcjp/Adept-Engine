#pragma once

class GPUPerformanceTest;
namespace EGPUPerformanceMetrics
{
	enum Type
	{
		PrimitveThoughput,
		Depththoughput,
		ComputeShadeSpeed,
		PixelShadeSpeed,
		AsyncComputeSpeed,
		ParticleComputeTest,
		RayIntersectionSpeed,
		Limit
	};
}
struct TestResultsData
{
	float TimeTaken = 0.0f;
	int ItemsProcessed = 0;
	bool PassedTest = false;
};
class GPUPerformanceTestManager
{
public:
	GPUPerformanceTestManager();
	~GPUPerformanceTestManager();
	void Init();
	void ExecuteAllTests();
	void GatherResults();
	void DestoryTests();
private:
	struct GPUTestSet
	{
		GPUPerformanceTest* Tests[EGPUPerformanceMetrics::Limit] = {0};
		DeviceContext* Device = nullptr;
		void ExecuteAllTests();
		void GatherResults();
		void Init();
		void Destroy();
	};
	GPUTestSet TestSet[MAX_GPU_DEVICE_COUNT];
};

