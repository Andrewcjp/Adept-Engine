#include "NVAPIManager.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "Core/Performance/PerfManager.h"
#include "nvapi.h"

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
		Log::LogMessage("NvAPI_Initialize Failed Code: " + std::to_string(ret), Log::Warning);
		IsOnline = false;
		return; // Initialization failed
	}

	ret = NvAPI_EnumPhysicalGPUs(GPUHandles, &GPUCount);
	if (ret != NVAPI_OK)
	{
		Log::LogMessage("EnumPhysicalGPUs Failed", Log::Warning);
		IsOnline = false;
		return;
	}
	for (unsigned int i = 0; i < GPUCount; i++)
	{
		GpuData.push_back(std::vector<std::string>());
		std::string Data = "GPU_" + std::to_string(i);
		GpuData[i].push_back(Data);
		NvAPI_ShortString name;
		ret = NvAPI_GPU_GetFullName(GPUHandles[i], name);
		if (ret == NVAPI_OK)
		{
			std::string named = name;
			NvU32 LaneCount = 0;
			NvAPI_GPU_GetCurrentPCIEDownstreamWidth(GPUHandles[i], &LaneCount);
			named += "(x" + std::to_string(LaneCount) + ")";
			GpuData[i].push_back(named);
		}
		GpuData[i].resize(StaticProps + DynamicProps);
	}
	IsOnline = true;
	DECLARE_TIMER_GROUP(GROUP_CLOCKS, "GPU Data");
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		StatIds[i][Stats::GPU0_GRAPHICS_PC] = PerfManager::Get()->GetTimerIDByName("GPU" + std::to_string(i) + "_GRAPHICS_PC");
		PerfManager::Get()->AddTimer(StatIds[i][Stats::GPU0_GRAPHICS_PC], GROUP_CLOCKS);
		StatIds[i][Stats::GPU0_GRAPHICS_CLOCK] = PerfManager::Get()->GetTimerIDByName("GPU" + std::to_string(i) + "_GRAPHICS_CLOCK");
		PerfManager::Get()->AddTimer(StatIds[i][Stats::GPU0_GRAPHICS_CLOCK], GROUP_CLOCKS);
#if STATS
		PerfManager::Get()->GetTimerData(StatIds[i][Stats::GPU0_GRAPHICS_PC])->DirectUpdate = true;
		PerfManager::Get()->GetTimerData(StatIds[i][Stats::GPU0_GRAPHICS_PC])->HiddenFromDisplay = true;
		PerfManager::Get()->GetTimerData(StatIds[i][Stats::GPU0_GRAPHICS_CLOCK])->DirectUpdate = true;
		PerfManager::Get()->GetTimerData(StatIds[i][Stats::GPU0_GRAPHICS_CLOCK])->HiddenFromDisplay = true;
#endif
	}
#endif
}

void NVAPIManager::CheckSupport(ID3D12Device* D)
{
	NV_QUERY_SINGLE_PASS_STEREO_SUPPORT_PARAMS Par;
	Par.version = NV_QUERY_SINGLE_PASS_STEREO_SUPPORT_PARAMS_VER1;
	NvAPI_Status ret = NVAPI_OK;
	ret = NvAPI_D3D12_QuerySinglePassStereoSupport(D, &Par);
	ensure(ret == NVAPI_OK);
}

NVAPIManager::~NVAPIManager()
{
#if NVAPI_PRESENT 
	if (IsOnline)
	{
		NvAPI_Unload();
	}
#endif
}

void NVAPIManager::RenderGPUStats(int statx, int starty)
{
#if NVAPI_PRESENT
	if (!IsOnline)
	{
		return;
	}
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
			TextRenderer::instance->RenderFromAtlas(GpuData[x][i], (float)statx + x * Xsize, (float)starty - Ysize * i, 0.35f, Colours[x]);
		}
	}
#endif
}

void NVAPIManager::SampleClocks()
{
#if NVAPI_PRESENT 
	if (!IsOnline)
	{
		SampleData = "NO NV GPUS";
		return;
	}
	//KHZ
	unsigned int CoreClock = 0;
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
			PerfManager::Get()->UpdateStat(StatIds[i][Stats::GPU0_GRAPHICS_CLOCK], CoreClock / 1000.0f, 0);

		}
		GpuData[i][index] = Data;
		index++;
		ret = NvAPI_GPU_GetDynamicPstatesInfoEx(GPUHandles[i], &PstatesInfo);
		if (ret == NVAPI_OK)
		{
			int Utilzieation = PstatesInfo.utilization[NVAPI_GPU_UTILIZATION_DOMAIN_GPU].percentage;
			float PC = (float)Utilzieation / 80.0f;
			Colours[i] = glm::mix(glm::vec3(1, 0, 0), glm::vec3(1), PC);
			Data = ("Graphics: " + std::to_string(PstatesInfo.utilization[NVAPI_GPU_UTILIZATION_DOMAIN_GPU].percentage) + "%");
			PerfManager::Get()->UpdateStat(StatIds[i][Stats::GPU0_GRAPHICS_PC], (float)Utilzieation, 0);
		}
#if 1
		GpuData[i][index] = Data;
		index++;
		if (ret == NVAPI_OK)
		{
			Data = ("FrameBuffer: " + std::to_string(PstatesInfo.utilization[NVAPI_GPU_UTILIZATION_DOMAIN_FB].percentage) + "%");
		}
#endif
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
