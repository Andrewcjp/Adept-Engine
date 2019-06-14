#include "PerfManager.h"
#include "BenchMarker.h"
#include "Core/Utils/NVAPIManager.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHI.h"
#include <chrono>
#include <iomanip>
#include <time.h>

PerfManager* PerfManager::Instance;
bool PerfManager::PerfActive = true;
unsigned long PerfManager::get_nanos()
{
	return (unsigned long)std::chrono::high_resolution_clock::now().time_since_epoch().count();
}
long PerfManager::GetSeconds()
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return (long)ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}
PerfManager * PerfManager::Get()
{
	if (Instance == nullptr)
	{
		Instance = new PerfManager();
	}
	return Instance;
}

float PerfManager::EndAndLogTimer(std::string name)
{
	float time = PerfManager::Get()->EndSingleActionTimer(name);
	PerfManager::Get()->LogSingleActionTimer(name);
	PerfManager::Get()->FlushSingleActionTimer(name);
	return time;
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
		SafeDelete(Instance);
	}
}

PerfManager::PerfManager()
{
	Instance = this;
	ShowAllStats = true;
	NVApiManager = new NVAPIManager();
	Bencher = new BenchMarker();
	//Test();
}
void PerfManager::Test()
{
	StartSingleActionTimer("test");
	int testtime = 5000;
	PlatformApplication::Sleep(testtime);
	PlatformApplication::Sleep(10);
	float output = EndSingleActionTimer("test");
	ensure(output >= testtime);
	ensure(output <= testtime);
}
PerfManager::~PerfManager()
{
	SafeDelete(NVApiManager);
	SafeDelete(Bencher);
}

int PerfManager::AddTimer(const char * countername, const char* group)
{
	return AddTimer(GetTimerIDByName(countername), GetGroupId(group));
}

int PerfManager::AddTimer(const char * countername, int groupId)
{
	return AddTimer(GetTimerIDByName(countername), groupId);
}

int PerfManager::AddTimer(int id, int groupid)
{
#if	STATS
	if (AVGTimers.find(id) != AVGTimers.end())
	{
		return id;
	}
	AVGTimers.emplace(id, TimerData(GetTimerName(id), groupid));
	TimerOutput.emplace(id, 0.0f);
#endif
	return id;
}

void PerfManager::StartTimer(const char * countername)
{
#if	STATS
	if (Instance != nullptr && PerfActive)
	{
		StartTimer(Instance->GetTimerIDByName(countername));
		//PIXBeginEvent(0,countername);
	}
#endif
}
void PerfManager::EndTimer(const char * countername)
{
#if STATS
	if (Instance != nullptr&& PerfActive)
	{
		EndTimer(Instance->GetTimerIDByName(countername));
		//PIXEndEvent();
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
void PerfManager::RenderGpuData(int x, int y)
{
	if (Instance != nullptr)
	{
		Instance->NVApiManager->RenderGPUStats(x, y);
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
		AVGTimers.at(targetTimer).LastFrameUsed = RHI::GetFrameCount();
		AVGTimers.at(targetTimer).CallCount += 1;
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
	return 1.0f / AVGFrameTime;
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
	CPUAVG.Add(CPUTime);

}

void PerfManager::StartFrameTimer()
{
	FrameStart = get_nanos();
}

void PerfManager::EndFrameTimer()
{
	FrameTime = (float)((get_nanos() - FrameStart) / 1e9f);//in s
	SetDeltaTime(FrameTime);
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
		return Instance->CPUAVG.GetCurrentAverage();
	}
	return 0;
}

float PerfManager::GetDeltaTime()
{
	if (Instance != nullptr)
	{
		return Instance->ClampedDT;
	}
	return 0;
}

float PerfManager::GetDeltaTimeRaw()
{
	if (Instance != nullptr)
	{
		return Instance->DeltaTime;
	}
	return 0.0f;
}

void PerfManager::SetDeltaTime(float Time)
{
	if (Instance != nullptr)
	{
		Instance->DeltaTime = Time;
		Instance->ClampedDT = Time;
		Instance->ClampedDT = std::min(Instance->ClampedDT, (1.0f / 10.0f));
	}
}
void PerfManager::NotifyEndOfFrame(bool Final)
{
	//clear timers
	if (Instance != nullptr)
	{
		PerfManager::Instance->EndFrameTimer();
		if (Final)
		{
			Instance->UpdateStats();
			Instance->Internal_NotifyEndOfFrame();
			Instance->UpdateStatsTimer();
		}
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
		if (AVGTimers.find(it->first) == AVGTimers.end())
		{
			continue;
		}
		TimerData*  data = &AVGTimers.at(it->first);
		data->AVG->Add(it->second);
		if (data->MAXAVG->IsLastIndex())
		{
			data->MAXAVG->Add(0);
		}
		else
		{
			data->MAXAVG->Add(glm::max(it->second, data->MAXAVG->GetCurrentAverage()));
		}

		if (!data->DirectUpdate)
		{
			it->second = 0.0f;
		}
		else
		{
			data->AVG->clear();
			data->AVG->Add(it->second);
		}

		data->LastCallCount = data->CallCount;
		data->CallCount = 0;
	}
}

TimerData * PerfManager::GetTimerData(int id)
{
	if (AVGTimers.find(id) != AVGTimers.end())
	{
		return &AVGTimers.at(id);
	}
	return nullptr;
}

void PerfManager::DrawAllStats(int x, int y, bool IncludeGPUStats)
{
#if STATS
	DrawStatsGroup(x, y, "", IncludeGPUStats);
	for (std::map<std::string, int>::iterator it = GroupIDS.begin(); it != GroupIDS.end(); ++it)
	{
		DrawStatsGroup(x, y, it->first, IncludeGPUStats);
	}
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		RHI::GetDeviceContext(i)->TickTransferStats(true);
	}
#endif
}
void PerfManager::UpdateStatsTimer()
{
	CurrentSlowStatsUpdate += GetDeltaTime();
	if (CurrentSlowStatsUpdate > SlowStatsUpdateRate)
	{
		CurrentSlowStatsUpdate = 0;
		SampleSlowStats();
	}
	StatAccum += GetDeltaTime();
	if (StatAccum >= StatsTickRate || !LockStatsRate)
	{
		Capture = true;
		StatAccum = 0.0f;
	}
	else
	{
		Capture = false;
	}
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		RHI::GetDeviceContext(i)->TickTransferStats(false);
	}
}

void PerfManager::UpdateStats()
{
	if (!Capture)
	{
		return;
	}
#if STATS
	ClearStats();
	Bencher->TickBenchMarker();
#endif
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
		it->second.MaxTime = it->second.MAXAVG->GetCurrentAverage();
		if (Capture)
		{
			if (it->second.LastFrameUsed < RHI::GetFrameCount() - 60)
			{
				it->second.Active = false;
			}
		}
		if (DidJustReset)
		{
			it->second.AVG->clear();
			it->second.AVG->Add(it->second.Time);
		}
	}
	if (DidJustReset)
	{
		const float AVG = CPUAVG.GetCurrentAverage();
		CPUAVG.clear();
		CPUAVG.Add(AVG);
		FrameAccum = 0;
		FrameTimeAccum = 0.0f;
	}
	DidJustReset = false;
}
std::vector<TimerData*> PerfManager::GetAllGPUTimers(std::string group)
{
	std::vector<TimerData*> Output;
	const int GroupFilterId = GetGroupId(group);
	for (std::map<int, TimerData>::iterator it = AVGTimers.begin(); it != AVGTimers.end(); ++it)
	{
		if (it->second.GroupId != GroupFilterId || !it->second.IsGPUTimer)
		{
			continue;
		}
		Output.push_back(&it->second);
	}
	return Output;
}

void PerfManager::StartBenchMark(std::string name)
{
	if (name.length() > 0)
	{
		Instance->Bencher->SetTestFileSufix(name);
	}
	Instance->ResetStats();
	Instance->Bencher->StartBenchMark();
}

void PerfManager::EndBenchMark()
{
	Instance->Bencher->StopBenchMark();
}

void PerfManager::AddToCountTimer(std::string name, int amout)
{
	TimerData* D = Instance->GetTimerData(Instance->GetTimerIDByName(name));
	if (D != nullptr && Instance->Capture)
	{
		D->CallCount += amout;
		D->Active = true;
		D->LastFrameUsed = RHI::GetFrameCount();
	}
}

void PerfManager::ResetStats()
{
	DidJustReset = true;
}

void PerfManager::DrawStatsGroup(int x, int& y, std::string GroupFilter, bool IncludeGPU)
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
		if (it->second.IsGPUTimer && !IncludeGPU)
		{
			continue;
		}
		if (IncludeGPU && !it->second.IsGPUTimer)
		{
			continue;
		}
		if (it->second.HiddenFromDisplay)
		{
			continue;
		}
		SortedTimers.push_back(&it->second);
	}
	if (SortedTimers.size() == 0)
	{
		return;
	}
	struct less_than_key
	{
		bool operator() (const TimerData* struct1, const TimerData* struct2)
		{
			return (struct1->Time > struct2->Time);
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
		Textcontext->RenderFromAtlas(SortedTimers[i]->name, (float)x, (float)CurrentHeight, TextSize);
		std::stringstream stream;
		stream << std::fixed << std::setprecision(3) << SortedTimers[i]->Time << "ms";
		Textcontext->RenderFromAtlas(stream.str(), (float)(x + ColWidth), (float)CurrentHeight, TextSize);
		stream.str("");
		stream << std::fixed << std::setprecision(3) << " Max:" << SortedTimers[i]->MaxTime << "ms ";
		Textcontext->RenderFromAtlas(stream.str(), (float)(x + ColWidth * 1.4f), (float)CurrentHeight, TextSize);
		stream.str("");
		if (!SortedTimers[i]->IsGPUTimer)
		{
			stream << SortedTimers[i]->LastCallCount;
			Textcontext->RenderFromAtlas(stream.str(), (float)(x + ColWidth * 2.0), (float)CurrentHeight, TextSize);
		}
		CurrentHeight -= Height;
	}
	CurrentHeight -= 10;
	y = CurrentHeight;
#endif
}

void PerfManager::UpdateStat(int id, float newtime, float GPUOffsetToMain, bool Direct /*= false*/)
{
	if (TimerOutput.find(id) != TimerOutput.end())
	{
		TimerData* data = PerfManager::Instance->GetTimerData(id);
		if (data != nullptr)
		{
			data->Active = true;
			data->LastFrameUsed = RHI::GetFrameCount();
			data->GPUStartOffset = GPUOffsetToMain;
			data->DirectUpdate = Direct;
		}
		TimerOutput.at(id) = glm::abs(newtime);
	}
	else
	{
		TimerOutput.emplace(id, 0.0f);
	}
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

void PerfManager::StartSingleActionTimer(std::string Name)
{
	if (SingleActionTimers.find(Name) != SingleActionTimers.end())
	{
		SingleActionTimers.at(Name) = GetSeconds();
	}
	else
	{
		SingleActionTimers.emplace(Name, GetSeconds());
		SingleActionTimersAccum.emplace(Name, 0.0f);
	}
}

float PerfManager::EndSingleActionTimer(std::string Name)
{
	if (SingleActionTimers.find(Name) != SingleActionTimers.end())
	{
		float TimeInMS = (float)(GetSeconds() - SingleActionTimers.at(Name)) /*/ TimeMS*/;
		SingleActionTimersAccum.at(Name) += TimeInMS;
		return TimeInMS;
	}
	else
	{
		SingleActionTimers.emplace(Name, 0);
		SingleActionTimersAccum.emplace(Name, 0.0f);
	}
	return 0.0f;
}

void PerfManager::FlushSingleActionTimers()
{
	for (std::map<std::string, float>::iterator it = SingleActionTimersAccum.begin(); it != SingleActionTimersAccum.end(); ++it)
	{
		it->second = 0.0f;
	}
}

void PerfManager::LogSingleActionTimers()
{
	for (std::map<std::string, float>::iterator it = SingleActionTimersAccum.begin(); it != SingleActionTimersAccum.end(); ++it)
	{
		Log::OutS << "Timer " << it->first << " Took " << it->second << "ms" << Log::OutS;
	}
}
void PerfManager::LogSingleActionTimer(std::string name)
{
	if (SingleActionTimersAccum.find(name) != SingleActionTimersAccum.end())
	{
		Log::OutS << "Timer " << name << " Took " << SingleActionTimersAccum.at(name) << "ms" << Log::OutS;
	}
}
void PerfManager::FlushSingleActionTimer(std::string name)
{
	if (SingleActionTimersAccum.find(name) != SingleActionTimersAccum.end())
	{
		SingleActionTimersAccum.at(name) = 0.0f;
	}
}

void PerfManager::WriteLogStreams(bool UseRaw)
{
	if (!Capture)
	{
		return;
	}
	for (std::map<int, TimerData>::iterator it = AVGTimers.begin(); it != AVGTimers.end(); ++it)
	{
		if (!it->second.Active)
		{
			continue;
		}
		if (UseRaw)
		{
			Bencher->WriteStat(it->first, it->second.AVG->GetRaw());
		}
		else
		{
			Bencher->WriteStat(it->first, it->second.Time);
		}
	}
	Bencher->WriteCoreStat(ECoreStatName::FrameTime, GetAVGFrameTime() * 1000);
	Bencher->WriteCoreStat(ECoreStatName::FrameRate, GetAVGFrameRate());
	Bencher->WriteCoreStat(ECoreStatName::CPU, GetCPUTime());
	Bencher->WriteCoreStat(ECoreStatName::GPU, GetGPUTime());
}

PerfManager::ScopeStartupCounter::ScopeStartupCounter(const char* name)
{
	Name = name;
	Get()->StartSingleActionTimer(name);
}

PerfManager::ScopeStartupCounter::~ScopeStartupCounter()
{
	Get()->EndSingleActionTimer(Name);
}
