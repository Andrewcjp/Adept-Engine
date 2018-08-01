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
#include "RHI/RHI.h"
#include "RHI/RenderAPIs/D3D12/D3D12TimeManager.h"
#include "RHI/DeviceContext.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "Core/Utils/NVAPIManager.h"
#include <algorithm>
PerfManager* PerfManager::Instance;
bool PerfManager::PerfActive = true;
long PerfManager::get_nanos()
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

PerfManager * PerfManager::Get()
{
	if (Instance == nullptr)
	{
		Instance = new PerfManager();
	}
	return Instance;
}

void PerfManager::StartPerfManager()
{
	if (Instance == nullptr)
	{
		Instance = new PerfManager();
	}
}

void PerfManager::ShutdownPerfManager()
{
	if (Instance != nullptr)
	{
		delete Instance;
	}
}

PerfManager::PerfManager()
{
	ShowAllStats = true;
	NVApiManager = new NVAPIManager();
}

PerfManager::~PerfManager()
{
	delete NVApiManager;
}

void PerfManager::AddTimer(const char * countername, const char* group)
{
	AddTimer(GetTimerIDByName(countername), GetGroupId(group));
}

void PerfManager::AddTimer(int id, int groupid)
{
	TimerData Data;
	Data.AVG = new MovingAverage(AvgCount);
	Data.name = GetTimerName(id);
	Data.GroupId = groupid;
	AVGTimers.emplace(id, Data);
	TimerOutput.emplace(id, 0.0f);
}

void PerfManager::StartTimer(const char * countername)
{
#if	STATS
	if (Instance != nullptr && PerfActive)
	{
		StartTimer(Instance->GetTimerIDByName(countername));
	}
#endif
}
void PerfManager::EndTimer(const char * countername)
{
#if STATS
	if (Instance != nullptr&& PerfActive)
	{
		EndTimer(Instance->GetTimerIDByName(countername));
	}
#endif
}

void PerfManager::StartTimer(int Counterid)
{
#if STATS
	if (Instance != nullptr&& PerfActive)
	{
		Instance->InStartTimer(Counterid);
	}
#endif
}

void PerfManager::EndTimer(int StatId)
{
#if STATS
	if (Instance != nullptr&& PerfActive)
	{
		Instance->InEndTimer(StatId);
	}
#endif
}
void PerfManager::RenderGpuData(int x,int y)
{
	if (Instance != nullptr)
	{
		Instance->NVApiManager->RenderGPUStats(x,y);
	}
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

int PerfManager::GetGroupId(std::string name)
{
	if (name.empty())
	{
		return -1;
	}
	if (GroupIDS.find(name) == GroupIDS.end())
	{
		GroupIDS.emplace(name, NextGroupId);
		NextGroupId++;
	}
	return GroupIDS.at(name);
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


void PerfManager::InStartTimer(int targetTimer)
{
	if (!Capture)
	{
		return;
	}
	if (TimersStartStamps.find(targetTimer) != TimersStartStamps.end())
	{
		TimersStartStamps.at(targetTimer) = get_nanos();
		AVGTimers.at(targetTimer).Active = true;
	}
	else
	{
		TimersStartStamps.emplace(targetTimer, 0);
		TimerOutput.emplace(targetTimer, 0.0f);
		AddTimer(GetTimerName(targetTimer).c_str(), "");
	}
}

void PerfManager::InEndTimer(int targetTimer)
{
	if (!Capture)
	{
		return;
	}

	if (TimersStartStamps.find(targetTimer) != TimersStartStamps.end())
	{
		if (TimersEndStamps.find(targetTimer) != TimersEndStamps.end())
		{
			TimersEndStamps.at(targetTimer) = get_nanos();
		}
		else
		{
			TimersEndStamps.emplace(targetTimer, 0);
			return;
		}
		const float Calctime = (float)((get_nanos() - TimersStartStamps.at(targetTimer)) / TimeMS);;//in ms;
		TimerOutput.at(targetTimer) += Calctime;

	}
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
	return 1.0f/AVGFrameTime;
}

float PerfManager::GetAVGFrameTime() const
{
	return AVGFrameTime;
}

std::string PerfManager::GetAllTimers()
{
	std::stringstream stream;
	if (ShowAllStats)
	{
		stream << std::fixed << std::setprecision(3) << "Stats: ";
		for (std::map<int, TimerData>::iterator it = AVGTimers.begin(); it != AVGTimers.end(); ++it)
		{
			stream << GetTimerName(it->first) << ": " << it->second.AVG->GetCurrentAverage() << "ms ";
		}
	}
	return stream.str();
}

void PerfManager::StartCPUTimer()
{
	CPUstart = get_nanos();
}

void PerfManager::EndCPUTimer()
{
	CPUTime = (float)((get_nanos() - CPUstart) / 1e6f);//in ms
	StatAccum += CPUTime;
	CPUAVG.Add(CPUTime);
	if (StatAccum > StatsUpdateSpeed)
	{
		Capture = true;
		StatAccum = 0.0f;
	}
	else
	{
		Capture = false;
	}
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
		return RHI::GetDeviceContext(0)->GetTimeManager()->GetTotalTime();
	}
	return 0.0f;
}

float PerfManager::GetCPUTime()
{
	if (Instance != nullptr)
	{
		return Instance->CPUAVG.GetCurrentAverage();//Instance->CPUTime;
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

void PerfManager::SetDeltaTime(float Time)
{
	if (Instance != nullptr)
	{
		Instance->FrameTime = Time;
	}
}

void PerfManager::NotifyEndOfFrame()
{
	//clear timers
	if (Instance != nullptr)
	{
		Instance->Internal_NotifyEndOfFrame();
		Instance->UpdateStats();
	}
}

void PerfManager::Internal_NotifyEndOfFrame()
{
	if (!Capture)
	{
		return;
	}
	for (std::map<int, float>::iterator it = TimerOutput.begin(); it != TimerOutput.end(); ++it)
	{
		AVGTimers.at(it->first).AVG->Add(it->second);
		it->second = 0.0f;
	}
}

PerfManager::TimerData * PerfManager::GetTimerData(int id)
{
	if (AVGTimers.find(id) != AVGTimers.end())
	{
		return &AVGTimers.at(id);
	}
	return nullptr;
}

void PerfManager::DrawAllStats(int x, int y)
{
#if STATS
	DrawStatsGroup(x, y, "");
	for (std::map<std::string, int>::iterator it = GroupIDS.begin(); it != GroupIDS.end(); ++it)
	{
		DrawStatsGroup(x, y, it->first);
	}
#endif
}

void PerfManager::UpdateStats()
{
	for (std::map<int, TimerData>::iterator it = AVGTimers.begin(); it != AVGTimers.end(); ++it)
	{
		it->second.Time = it->second.AVG->GetCurrentAverage();
	}
	CurrentSlowStatsUpdate += GetDeltaTime();
	if (CurrentSlowStatsUpdate > SlowStatsUpdateRate)
	{
		CurrentSlowStatsUpdate = 0;
		SampleSlowStats();
	}
}

void PerfManager::SampleSlowStats()
{ 
	NVApiManager->SampleClocks();
}

void PerfManager::ClearStats()
{
	for (std::map<int, TimerData>::iterator it = AVGTimers.begin(); it != AVGTimers.end(); ++it)
	{
		it->second.Time = it->second.AVG->GetCurrentAverage();
	}
}

void PerfManager::DrawStatsGroup(int x, int& y, std::string GroupFilter)
{
#if STATS

	int CurrentHeight = y;
	TextRenderer* Textcontext = TextRenderer::instance;
	SortedTimers.clear();
	const int GroupFilterId = GetGroupId(GroupFilter);
	for (std::map<int, TimerData>::iterator it = AVGTimers.begin(); it != AVGTimers.end(); ++it)
	{
		if (GroupFilterId != -1)
		{
			if (it->second.GroupId != GroupFilterId)
			{
				continue;
			}
		}
		else
		{
			//ignore Stats in a group
			if (it->second.GroupId != -1)
			{
				continue;
			}
		}
		if (!it->second.Active)
		{
			continue;
		}
		SortedTimers.push_back(it->second);
	}
	struct less_than_key
	{
		bool operator() (TimerData& struct1, TimerData& struct2)
		{
			return (struct1.Time > struct2.Time);
		}
	};
	std::sort(SortedTimers.begin(), SortedTimers.end(), less_than_key());
	//title
	if (!GroupFilter.empty())
	{
		Textcontext->RenderFromAtlas(GroupFilter, (float)x, (float)CurrentHeight, TextSize);
		CurrentHeight -= Height;
	}
	for (int i = 0; i < SortedTimers.size(); i++)
	{
		Textcontext->RenderFromAtlas(SortedTimers[i].name, (float)x, (float)CurrentHeight, TextSize);
		std::stringstream stream;
		stream << std::fixed << std::setprecision(3) << SortedTimers[i].Time << "ms ";
		Textcontext->RenderFromAtlas(stream.str(), (float)(x + ColWidth), (float)CurrentHeight, TextSize);
		CurrentHeight -= Height;
	}
	y = CurrentHeight;
#endif
}

void PerfManager::UpdateGPUStat(int id, float newtime)
{
	if (TimerOutput.find(id) != TimerOutput.end())
	{
		PerfManager::TimerData* data = PerfManager::Instance->GetTimerData(id);
		if (data != nullptr)
		{
			data->Active = true;
		}
		TimerOutput.at(id) = newtime;
	}
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
	std::stringstream Out;
	float ToMB = (1.0f / 1000000.0f);
	Out << "Draw calls " << PerfManager::Engine::CompRegistry->GetValue(OGLBatch) << " Idle " /*<< std::fixed << std::setprecision(1)*/ << PerfManager::Engine::CompRegistry->GetValue(OGLMem)*ToMB;
	//<< "MB (" << PerfManager::Instance->GetValue(OGLTextureMem)*ToMB << "MB) ";

	std::string out = Out.str();
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

PerfManager::ScopeCycleCounter::ScopeCycleCounter(const char * Name)
{
	StatId = Instance->GetTimerIDByName(Name);
	PerfManager::StartTimer(StatId);
}

PerfManager::ScopeCycleCounter::ScopeCycleCounter(const char * Name, const char * group)
{
	StatId = Instance->GetTimerIDByName(Name);
	GroupID = Instance->GetGroupId(group);
	Instance->AddTimer(StatId, GroupID);
	PerfManager::StartTimer(StatId);
}

PerfManager::ScopeCycleCounter::~ScopeCycleCounter()
{
	PerfManager::EndTimer(StatId);
}
