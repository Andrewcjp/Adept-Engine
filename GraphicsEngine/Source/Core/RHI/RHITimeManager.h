#pragma once

#include "RHI_inc_fwd.h"
#include "RHITypes.h"
class RHI_API RHITimeManager
{
public:
	RHITimeManager(class DeviceContext* context);
	virtual ~RHITimeManager();
	virtual void UpdateTimers() = 0;
	virtual std::string GetTimerData() = 0;
	virtual void SetTimerName(int index, std::string Name, ECommandListType::Type type) = 0;
	virtual void StartTotalGPUTimer(RHICommandList * ComandList) = 0;
	virtual void StartTimer(RHICommandList * ComandList, int index) = 0;
	virtual void EndTimer(RHICommandList * ComandList, int index) = 0;
	virtual void EndTotalGPUTimer(RHICommandList * ComandList) = 0;
	virtual float GetTotalTime() = 0;
	virtual void ResolveTimeHeaps(RHICommandList * CommandList) = 0;
	virtual void ResolveCopyTimeHeaps(RHICommandList * ComandList) = 0;
protected:
	DeviceContext * Context = nullptr;
	const int CopyOffset = EGPUTIMERS::LIMIT;
};

