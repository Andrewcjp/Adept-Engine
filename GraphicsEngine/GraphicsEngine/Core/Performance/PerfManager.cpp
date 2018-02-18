#include "stdafx.h"
#include "PerfManager.h"
#if BUILD_WITH_NVPERFKIT
#ifndef NVPM_INITGUID
#include "NVPerfKit/NvPmApi.Manager.h"
#include "SharedHeader.h"
#endif
#endif
#include <iomanip>
#include <time.h>
#include "../RHI/RHI.h"
PerfManager* PerfManager::Instance;
bool PerfManager::PerfActive = true;
long PerfManager::get_nanos()
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}
void PerfManager::StartPerfManager()
{
	if (Instance == nullptr)
	{
		Instance = new PerfManager();
	}
}

PerfManager::PerfManager()
{
	if (RHI::GetType() == RenderSystemOGL)
	{
		glGenQueries(2, queryID);
	}
}


PerfManager::~PerfManager()
{
}


void PerfManager::StartTimer(const char * countername)
{
#if	STATS
	if (Instance != nullptr && PerfActive)
	{
		Instance->InStartTimer(countername);
	}
#endif
}
void PerfManager::EndTimer(const char * countername)
{
#if STATS
	if (Instance != nullptr&& PerfActive)
	{
		Instance->InEndTimer(countername);
	}
#endif
}
int PerfManager::GetTimerIDByName(std::string name)
{
	if (TimerIDs.find(name) == TimerIDs.end())
	{
		TimerIDs.emplace(name, NextId);
		NextId++;
	}
	return TimerIDs.at(name);
}
std::string PerfManager::GetTimerName(int id)
{
#if STATS
	for (std::map<std::string, int >::iterator it = TimerIDs.begin(); it != TimerIDs.end(); ++it)
	{
		if (it->second == id)
		{
			return it->first;
		}
	}
#endif
	return std::string();
}

void PerfManager::InStartTimer(const char * countername)
{
	int targetTimer = GetTimerIDByName(countername);
	if (Timers.find(targetTimer) != Timers.end())
	{
		Timers.at(targetTimer) = (float)get_nanos();
	}
	else
	{
		Timers.emplace(targetTimer, 0.0f);
		TimerOutput.emplace(targetTimer, 0.0f);
	}
}

void PerfManager::InEndTimer(const char * countername)
{
	int targetTimer = GetTimerIDByName(countername);
	if (Timers.find(targetTimer) != Timers.end())
	{
		Timers.at(targetTimer) = (float)((get_nanos() - Timers.at(targetTimer)) / TimeMS);;//in ms;
		TimerOutput.at(targetTimer) = Timers.at(targetTimer);
	}
}

float PerfManager::GetTimerValue(const char * countername)
{
	if (Timers.size() != 0)
	{
		return Timers.at(GetTimerIDByName(countername));
	}
	return 0.0f;
}

float PerfManager::GetAVGFrameRate()
{
	FrameAccum++;
	float timesincestat = (((float)(clock()) / CLOCKS_PER_SEC));//in s
	FrameTimeAccum += FrameTime;
	if (timesincestat > fpsnexttime)
	{
		CurrentAVGFps = (FrameAccum / (FpsPollSpeed));
		fpsnexttime += FpsPollSpeed;
		AVGFrameTime = FrameTimeAccum / FrameAccum;
		FrameAccum = 0;
		FrameTimeAccum = 0.0f;
	}
	return CurrentAVGFps;
}
float PerfManager::GetAVGFrameTime()
{
	return AVGFrameTime;
}
std::string PerfManager::GetAllTimers()
{
	std::stringstream stream;
	if (ShowAllStats)
	{
		stream << std::fixed << std::setprecision(3) << "Stats: ";
		for (std::map<int, float>::iterator it = TimerOutput.begin(); it != TimerOutput.end(); ++it)
		{
			stream << GetTimerName(it->first) << ": " << it->second << "ms ";
		}
	}

	return stream.str();
}
void PerfManager::StartGPUTimer()
{
	if (RHI::GetType() == RenderSystemOGL)
	{
		if (!WaitGPUTimerQuerry)
		{
			glQueryCounter(queryID[0], GL_TIMESTAMP);
		}
	}
}
void PerfManager::EndGPUTimer()
{
	if (RHI::GetType() == RenderSystemOGL)
	{
		if (!WaitGPUTimerQuerry)
		{
			glQueryCounter(queryID[1], GL_TIMESTAMP);
			WaitGPUTimerQuerry = true;
		}
		if (WaitGPUTimerQuerry)
		{
			glGetQueryObjectiv(queryID[1],
				GL_QUERY_RESULT_AVAILABLE,
				&stopTimerAvailable);
		}

		if (stopTimerAvailable)
		{
			GLuint64 startTime, stopTime;
			glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &startTime);
			glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &stopTime);
			GPUTime = ((stopTime - startTime) / 1e6f);
			WaitGPUTimerQuerry = false;
		}
	}
}

void PerfManager::StartCPUTimer()
{
	CPUstart = get_nanos();
}

void PerfManager::EndCPUTimer()
{
	CPUTime = (float)((get_nanos() - CPUstart) / 1e6f);//in ms
}

void PerfManager::StartFrameTimer()
{
	FrameStart = get_nanos();
}

void PerfManager::EndFrameTimer()
{
	FrameTime = (float)((get_nanos() - FrameStart) / 1e9f);//in s
}

float PerfManager::GetGPUTime()
{
	if (Instance != nullptr)
	{
		return Instance->GPUTime;
	}
	return 0.0f;
}
float PerfManager::GetCPUTime()
{
	if (Instance != nullptr)
	{
		return Instance->CPUTime;
	}
	return 0;
}

float PerfManager::GetDeltaTime()
{
	if (Instance != nullptr)
	{
		return Instance->FrameTime;
	}
	return 0;
}
//nv perf kit
//-------------------------------------------------------------------------------------------------------------------------------
bool PerfManager::InitNV()
{
#if BUILD_WITH_NVPERFKIT
	DidInitNVCounters = true;
	if (S_NVPMManager.Construct(L"C:/Users/AANdr/Dropbox/Engine/Engine/GraphicsEngine/x64/Debug/NvPmApi.Core.dll") != S_OK)
	{
		return false; // This is an error condition
	}
	NVPMRESULT nvResult;
	if ((nvResult = GetNvPmApi()->Init()) != NVPM_OK)
	{
		return false; // This is an error condition
	}

	if ((nvResult = GetNvPmApi()->CreateContextFromOGLContext((uint64_t)wglGetCurrentContext(), &hNVPMContext)) != NVPM_OK)
	{
		return false; // This is an error condition
	}
	//"OGL frame time"
	if ((nvResult = GetNvPmApi()->AddCounterByName(hNVPMContext, OGLBatch)) != NVPM_OK) { __debugbreak(); }
	if ((nvResult = GetNvPmApi()->AddCounterByName(hNVPMContext, OGLMem)) != NVPM_OK) { __debugbreak(); }
	if ((nvResult = GetNvPmApi()->AddCounterByName(hNVPMContext, OGLTextureMem)) != NVPM_OK) { __debugbreak(); }
#endif
	return true;
}

void PerfManager::SampleNVCounters()
{
#if BUILD_WITH_NVPERFKIT
	if (DidInitNVCounters)
	{
		NVPMUINT nCount = 0;
		if ((GetNvPmApi()->Sample(hNVPMContext, NULL, &nCount)) != NVPM_OK)
		{
			__debugbreak();
		}
	}
#endif
}
std::string PerfManager::GetCounterData()
{
#if BUILD_WITH_NVPERFKIT
	if (!DidInitNVCounters)
	{
		return std::string();
	}
	std::stringstream stream;
	float ToMB = (1.0f / 1000000.0f);
	stream << "Draw calls " << PerfManager::Instance->GetValue(OGLBatch) << " Idle " /*<< std::fixed << std::setprecision(1)*/ << PerfManager::Instance->GetValue(OGLMem)*ToMB;
	//<< "MB (" << PerfManager::Instance->GetValue(OGLTextureMem)*ToMB << "MB) ";

	std::string out = stream.str();
	return out;
#else
	return std::string();
#endif
}
uint64_t PerfManager::GetValue(const char * countername)
{
#if BUILD_WITH_NVPERFKIT
	if (!DidInitNVCounters)
	{
		return 0;
	}

	NVPMUINT64 value = 0;
	NVPMUINT64 cycles = 0;
	NVPMRESULT nvResult;
	NVPMCounterID id = 0;
	NVPMUINT8  overflow = 0;
	(GetNvPmApi()->GetCounterIDByContext(hNVPMContext, countername, &id));
	//NVPMCHECKCONTINUE(GetNvPmApi()->GetCounterAttribute(id, NVPMA_COUNTER_VALUE_TYPE, &type));
	//GetNvPmApi()->GetCounterIDByContext(hNVPMContext, OGLFrameTime, &id);
	//if ((nvResult = GetNvPmApi()->GetCounterValueByName((NVPMContext)hNVPMContext, OGLFrameTime, 0, &value, &cycles)) != NVPM_OK)
	//{
	//	__debugbreak();
	//}
	if ((nvResult = GetNvPmApi()->GetCounterValueUint64(hNVPMContext, id, 0, &value, &cycles, &overflow)) != NVPM_OK)
	{
		//__debugbreak();
	}
	/*if ((nvResult = GetNvPmApi()->GetCounterValueUint64(hNVPMContext, id, 0, &value, &cycle, &overflow)) != NVPM_OK)
	{
	__debugbreak();
	}*/
	return value;
#else
	return 0;
#endif

}