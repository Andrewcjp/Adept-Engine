#pragma once
//This might not be supported by every complier version
//detect if present as NVAPI is not distributable
#if __has_include("include/nvapi/nvapi.h")
#define NVAPI_PRESENT 1
#else 
#define NVAPI_PRESENT 0
#endif

#if NVAPI_PRESENT 
#pragma comment(lib, "nvapi64.lib")
#include "include/nvapi/nvapi.h"
#endif
class NVAPIManager
{
public:
	NVAPIManager();
	~NVAPIManager();
	void RenderGPUStats(int x, int y);
	void SampleClocks();
private:
	bool IsOnline = false;
	int StaticProps = 2;
	int DynamicProps = 4;
	std::string SampleData;
	std::vector<std::vector<std::string>> GpuData;
#if NVAPI_PRESENT 
	NvU32 GPUCount = 0;
	NvPhysicalGpuHandle GPUHandles[NVAPI_MAX_PHYSICAL_GPUS];
#endif
};

