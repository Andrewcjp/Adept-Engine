#include "Stdafx.h"
#include "NVAPIManager.h"
#include "Core/Asserts.h"
#include "Core/Platform/Logger.h"
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
	for (unsigned int i = 0; i < GPUCount; i++)
	{
		std::string Data = "GPU_" + std::to_string(i) + ": ";
		NV_GPU_CLOCK_FREQUENCIES clkFreqs = {};
		clkFreqs.version = NV_GPU_CLOCK_FREQUENCIES_VER_2;
		NvAPI_Status ret = NvAPI_GPU_GetAllClockFrequencies(GPUHandles[i], &clkFreqs);
		if (ret == NVAPI_OK)
		{
			CoreClock = clkFreqs.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency;
			Data.append("Core: " + std::to_string(CoreClock / 1000));
		}	
		Data.append(" ");
		SampleData.append(Data);
	}
#else 
	SampleData = "NVAPI Not Present";
#endif
}
