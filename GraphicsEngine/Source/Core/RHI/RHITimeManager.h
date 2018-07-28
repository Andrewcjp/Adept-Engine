#pragma once
#include "RHI_fwd.h"
class RHITimeManager
{
public:
	RHITimeManager(class DeviceContext* context);
	~RHITimeManager();
	virtual void UpdateTimers() = 0;
	virtual std::string GetTimerData() = 0;
	virtual void SetTimerName(int index, std::string Name) = 0;
	virtual void StartTotalGPUTimer(RHICommandList * ComandList) = 0;
	virtual void StartTimer(RHICommandList * ComandList, int index) = 0;
	virtual void EndTimer(RHICommandList * ComandList, int index) = 0;
	virtual void EndTotalGPUTimer(RHICommandList * ComandList) = 0;
	virtual float GetTotalTime() = 0;
protected:
	DeviceContext * Context = nullptr;
};

