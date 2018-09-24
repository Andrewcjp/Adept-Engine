#pragma once
#include "RHI/RHITimeManager.h"
class VkanTimeManager:public RHITimeManager
{
public:
	VkanTimeManager();
	~VkanTimeManager();

	// Inherited via RHITimeManager
	virtual void UpdateTimers() override;
	virtual std::string GetTimerData() override;
	virtual void SetTimerName(int index, std::string Name) override;
	virtual void StartTotalGPUTimer(RHICommandList * ComandList) override;
	virtual void StartTimer(RHICommandList * ComandList, int index) override;
	virtual void EndTimer(RHICommandList * ComandList, int index) override;
	virtual void EndTotalGPUTimer(RHICommandList * ComandList) override;
	virtual float GetTotalTime() override;
	virtual void ResolveCopyTimeHeaps(RHICommandList * ComandList) override;
};

