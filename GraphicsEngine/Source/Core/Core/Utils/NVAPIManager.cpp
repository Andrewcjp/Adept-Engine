#include "Stdafx.h"
#include "NVAPIManager.h"
#include "Core/Asserts.h"
#include "Core/Platform/Logger.h"
#include "Rendering/Renderers/TextRenderer.h"

#define NVAPI_GPU_UTILIZATION_DOMAIN_GPU 0
#define NVAPI_GPU_UTILIZATION_DOMAIN_FB  1
#define NVAPI_GPU_UTILIZATION_DOMAIN_VID 2
#define NVAPI_GPU_UTILIZATION_DOMAIN_BUS 3

NVAPIManager::NVAPIManager()
{
#if NVAPI_PRESENT 
	NvAPI_Status ret = NVAPI_OK;

	ret = NvAPI_Initialize();
	if (ret != NVAPI_OK)
	{
		printf("NvAPI_Initialize() failed = 0x%x", ret);
		return; // Initialization failed
	}

	ret = NvAPI_EnumPhysicalGPUs(GPUHandles, &GPUCount);
	ensure(ret == NVAPI_OK);
	for (unsigned int i = 0; i < GPUCount; i++)
	{
		GpuData.push_back(std::vector<std::string>());
		std::string Data = "GPU_" + std::to_string(i);
		GpuData[i].push_back(Data);
		NvAPI_ShortString name;
		NvAPI_Status ret = NvAPI_GPU_GetFullName(GPUHandles[i], name);
		if (ret == NVAPI_OK)
		{
			GpuData[i].push_back(name);
		}
		GpuData[i].resize(StaticProps + DynamicProps);
	}
#endif
}


NVAPIManager::~NVAPIManager()
{
#if NVAPI_PRESENT 
	NvAPI_Unload();
#endif
}

std::string NVAPIManager::GetClockData()
{
	return SampleData;
}

void NVAPIManager::RenderGPUStats(int statx, int starty)
{
	int Ysize = 20;
	int Xsize = 200;
	if (GpuData[0].size() == 0)
	{
		return;
	}
	for (unsigned int x = 0; x < GPUCount; x++)
	{
		for (int i = 0; i < GpuData[x].size(); i++)
		{
			TextRenderer::instance->RenderFromAtlas(GpuData[x][i], (float)statx + x * Xsize, starty - Ysize * i, 0.35f);
		}
	}

}

void NVAPIManager::SampleClocks()
{
#if NVAPI_PRESENT 

	//KHZ
	unsigned int CoreClock = 0;
	unsigned int PROCESSORClock = 0;
	SampleData = "";
	if (GPUCount == 0)
	{
		SampleData = "NO NV GPUS";
		return;
	}
	NV_GPU_CLOCK_FREQUENCIES clkFreqs = {};
	clkFreqs.version = NV_GPU_CLOCK_FREQUENCIES_VER_2;
	NV_GPU_DYNAMIC_PSTATES_INFO_EX PstatesInfo = {};
	PstatesInfo.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
	const bool ShowTemp = true;
	NV_GPU_THERMAL_SETTINGS thermal = {};
	thermal.version = NV_GPU_THERMAL_SETTINGS_VER;
	for (unsigned int i = 0; i < GPUCount; i++)
	{
		int index = StaticProps;
		std::string Data = "";
		NvAPI_Status ret = NvAPI_GPU_GetAllClockFrequencies(GPUHandles[i], &clkFreqs);
		if (ret == NVAPI_OK)
		{
			CoreClock = clkFreqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency;
			Data = ("Core: " + std::to_string(CoreClock / 1000) + "MHz");
		}
		GpuData[i][index] = Data;
		index++;
		ret = NvAPI_GPU_GetDynamicPstatesInfoEx(GPUHandles[i], &PstatesInfo);
		if (ret == NVAPI_OK)
		{
			Data = ("Graphics: " + std::to_string(PstatesInfo.utilization[NVAPI_GPU_UTILIZATION_DOMAIN_GPU].percentage) + "%");			
		}
		GpuData[i][index] = Data;
		index++;
		if (ret == NVAPI_OK)
		{
			Data = ("Copy: " + std::to_string(PstatesInfo.utilization[NVAPI_GPU_UTILIZATION_DOMAIN_BUS].percentage) + "%");
		}
		GpuData[i][index] = Data;
		index++;
		if (ShowTemp)
		{
			ret = NvAPI_GPU_GetThermalSettings(GPUHandles[i], 0, &thermal);
			if (ret == NVAPI_OK)
			{
				Data = ("Core Temp " + std::to_string(thermal.sensor[0].currentTemp) + "C");
			}
			GpuData[i][index] = Data;
			index++;
		}
	}
#else 
	SampleData = "NVAPI Not Present";
#endif
}
