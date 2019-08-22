
#include "RHITimeManager.h"
#include "DeviceContext.h"
#include "Core\Performance\PerfManager.h"
static ConsoleVariable CaptureFullTimers("GPUPerf.PerDrawTimers", 0, ECVarType::ConsoleAndLaunch);
static ConsoleVariable Paused("GPUPerf.Pause", 0, ECVarType::ConsoleAndLaunch);
RHITimeManager::RHITimeManager(DeviceContext * context)
{
	Context = context;
	TimerNames.resize(EGPUTIMERS::LIMIT + EGPUCOPYTIMERS::LIMIT);
}

RHITimeManager::~RHITimeManager()
{}

ScopedGPUTimer::ScopedGPUTimer(RHICommandList* List, std::string Name, bool PerDraw /*= false*/)
{
#if PERDRAWTIMER
	if (!CaptureFullTimers.GetBoolValue() && PerDraw)
	{
		return;
	}
#endif
	cmdlist = List;
	GPUTimerId = List->GetDevice()->GetTimeManager()->GetGPUTimerId(Name);
	PerfManager::Get()->AddGPUTimer((Name + std::to_string(cmdlist->GetDevice()->GetDeviceIndex())).c_str(), PerfManager::Get()->GetGroupId("GPU_" + std::to_string(cmdlist->GetDevice()->GetDeviceIndex())));
	TimerData* D = PerfManager::Get()->GetTimerData(GPUTimerId);
	D->TimerType = List->GetListType();
	cmdlist->StartTimer(GPUTimerId);
}

ScopedGPUTimer::~ScopedGPUTimer()
{
	if (cmdlist != nullptr)
	{
		cmdlist->EndTimer(GPUTimerId);
	}
}

int RHITimeManager::GetGPUTimerId(std::string name)
{
	auto itor = GPUTimerMap.find(name);
	if (itor == GPUTimerMap.end())
	{
		AssignedIdCount++;
		GPUTimerMap.emplace(name, AssignedIdCount);
		TimerNames.push_back(name);
		return AssignedIdCount;
	}
	return itor->second;
}

GPUTimer * RHITimeManager::GetTimer(std::string name, DeviceContext * D)
{
	auto itor = Timers.find(std::make_pair(name, D));
	if (itor == Timers.end())
	{
		return nullptr;
	}
	return itor->second;
}

GPUTimer * RHITimeManager::GetOrCreateTimer(std::string name, DeviceContext * D, ECommandListType::Type type)
{
	GPUTimer* newtimer = GetTimer(name, D);
	if (newtimer == nullptr)
	{
		newtimer = new GPUTimer();
		newtimer->name = name;
		newtimer->Context = D;
		newtimer->Type = type;
		AddTimer(newtimer, name, D);
	}
	return newtimer;
}

std::string RHITimeManager::GetTimerName(int id)
{
	if (id < TimerNames.size())
	{
		return TimerNames[id];
	}
	return "";
}

std::vector<GPUTimerPair*> RHITimeManager::GetGPUTimers(ECommandListType::Type type)
{
	std::vector<GPUTimerPair*> out;
	for (auto itor = Timers.begin(); itor != Timers.end(); itor++)
	{
		if (itor->first.first != TimerNames[0])
		{
			if (itor->second->Type == type)
			{
				for (int i = 0; i < itor->second->NormalisedStamps.size(); i += 2)
				{
					GPUTimerPair* Pair = new GPUTimerPair();
					Pair->Stamps[0] = itor->second->NormalisedStamps[i]->GetCurrentAverage();
					Pair->Stamps[1] = itor->second->NormalisedStamps[i + 1]->GetCurrentAverage();
					Pair->Owner = itor->second;
					out.push_back(Pair);
				}
			}
		}
	}
	return out;
}

bool RHITimeManager::IsRunning()
{
	return !Paused.GetBoolValue();
}

void RHITimeManager::PushToPerfManager()
{
	if (!IsRunning())
	{
		return;
	}
	GPUTimer* MainTimer = GetTimer(TimerNames[0], Context);
	for (auto itor = Timers.begin(); itor != Timers.end(); itor++)
	{
		if (MainTimer->Stamps.size() > 1)
		{
			itor->second->Normalise(MainTimer->Stamps[0], MainTimer->Stamps[1]);
		}
	}
	ADNOP;
}

void RHITimeManager::AddTimer(GPUTimer * Data, std::string name, DeviceContext * D)
{
	Timers.emplace(std::make_pair(name, D), Data);
}

void GPUTimer::Clear()
{
	Stamps.clear();
}

void GPUTimer::AddResults(uint64_t start)
{
	Stamps.push_back(start);
	if (NormalisedStamps.size() < Stamps.size())
	{
		NormalisedStamps.push_back(new MovingAverage(50));
	}
}

void GPUTimer::Normalise(uint64_t StartStamp, uint64_t endstamp)
{
	for (int i = 0; i < Stamps.size(); i++)
	{
		uint64_t PreSclaaeV = Stamps[i] - StartStamp;
		uint64_t Delta = endstamp - StartStamp;
		float v = ((float)PreSclaaeV / (float)(Delta));
		if (isinf(v) || PreSclaaeV == 0 || Stamps[i] == 0)
		{
			continue;
		}
		NormalisedStamps[i]->Add(v);
	}
}
