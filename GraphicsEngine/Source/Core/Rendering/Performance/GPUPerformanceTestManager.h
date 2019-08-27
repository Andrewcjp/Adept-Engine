#pragma once

class GPUPerformanceTest;
namespace EGPUPerformanceMetrics
{
	enum Type
	{
		PrimitveThoughput,
		GeometryProcessingSpeed,
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
	void AnalysizeResults();
	void LogResults();
	void DestoryTests();
private:
	struct GPUTestSet
	{
		GPUPerformanceTest* Tests[EGPUPerformanceMetrics::Limit] = {0};
		DeviceContext* Device = nullptr;
		void ExecuteAllTests();
		void GatherResults();
		void LogResults(GPUTestSet* ZeroSet);
		void Init();
		void Destroy();
	};
	GPUTestSet TestSet[MAX_GPU_DEVICE_COUNT];
};

