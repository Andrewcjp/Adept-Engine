
#include "RHITimeManager.h"
#include "DeviceContext.h"
#include "Core\Performance\PerfManager.h"

RHITimeManager::RHITimeManager(DeviceContext * context)
{
	Context = context;
	TimerNames.resize(EGPUTIMERS::LIMIT + EGPUCOPYTIMERS::LIMIT);
}

RHITimeManager::~RHITimeManager()
{}

ScopedGPUTimer::ScopedGPUTimer(RHICommandList * List, std::string Name)
{
	cmdlist = List;
	GPUTimerId = List->GetDevice()->GetTimeManager()->GetGPUTimerId(Name);
	PerfManager::Get()->AddGPUTimer((Name + std::to_string(cmdlist->GetDevice()->GetDeviceIndex())).c_str(), PerfManager::Get()->GetGroupId("GPU_" + std::to_string(cmdlist->GetDevice()->GetDeviceIndex())));
	cmdlist->StartTimer(GPUTimerId);
}

ScopedGPUTimer::~ScopedGPUTimer()
{
	cmdlist->EndTimer(GPUTimerId);
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
