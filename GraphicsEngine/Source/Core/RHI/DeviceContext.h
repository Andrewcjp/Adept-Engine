#pragma once

#include "RHI/RHI_inc.h"
struct CapabilityData
{
	bool SupportsCopyTimeStamps = false;
};
class RHI_API DeviceContext
{
public:
	DeviceContext();
	virtual ~DeviceContext();

	virtual void ResetDeviceAtEndOfFrame() = 0;
	virtual void SampleVideoMemoryInfo() = 0;
	virtual std::string GetMemoryReport() = 0;
	virtual void MoveNextFrame() = 0;
	virtual void DestoryDevice() = 0;
	virtual void WaitForGpu() = 0;
	virtual void WaitForCopy() = 0;
	virtual void ResetSharingCopyList() = 0;
	virtual void NotifyWorkForCopyEngine() = 0;
	virtual void UpdateCopyEngine() = 0;
	virtual void ResetCopyEngine() = 0;

	virtual int GetDeviceIndex() = 0;
	virtual int GetCpuFrameIndex() = 0;
	virtual void GPUWaitForOtherGPU(DeviceContext * OtherGPU, DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue) = 0;
	int CurrentFrameIndex = 0;
	virtual void CPUWaitForAll() = 0;
	virtual void InsertGPUWait(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue) = 0;
	virtual void WaitForGPU(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue) = 0;
	const CapabilityData& GetCaps();
	RHICommandList* GetInterGPUCopyList();
	virtual class RHITimeManager* GetTimeManager() = 0;
protected:
	bool LogDeviceDebug = true;
	int DeviceIndex = 0;
	//copy queue management 
	bool CopyEngineHasWork = false;
	RHICommandList* InterGPUCopyList = nullptr;
	RHICommandList* GPUCopyList = nullptr;
	CapabilityData Caps_Data = CapabilityData();
};

