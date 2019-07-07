#pragma once

#include "RHI_inc_fwd.h"
#include "RHITypes.h"
class  RHITimeManager
{
public:
	RHI_API RHITimeManager(class DeviceContext* context);
	RHI_API virtual ~RHITimeManager();
	RHI_API virtual void UpdateTimers() = 0;
	RHI_API virtual void SetTimerName(int index, std::string Name, ECommandListType::Type type) = 0;
	RHI_API virtual void StartTotalGPUTimer(RHICommandList * ComandList) = 0;
	RHI_API virtual void StartTimer(RHICommandList * ComandList, int index) = 0;
	RHI_API virtual void EndTimer(RHICommandList * ComandList, int index) = 0;
	RHI_API virtual void EndTotalGPUTimer(RHICommandList * ComandList) = 0;
	RHI_API virtual float GetTotalTime() = 0;
	RHI_API int GetGPUTimerId(std::string name);
protected:
	DeviceContext * Context = nullptr;
	const int CopyOffset = EGPUTIMERS::LIMIT;
	std::map<std::string, int> GPUTimerMap;
	int AssignedIdCount = EGPUTIMERS::LIMIT + EGPUCOPYTIMERS::LIMIT-1;
	std::vector<std::string> TimerNames;
};

struct ScopedGPUTimer
{
public:
	ScopedGPUTimer(RHICommandList* List, std::string Name);
	~ScopedGPUTimer();
private:
	int GPUTimerId = 0;
	RHICommandList* cmdlist = nullptr;
};

#define DECALRE_SCOPEDGPUCOUNTER(list,name) ScopedGPUTimer PREPROCESSOR_JOIN(COUNTER,__LINE__)(list,name);
