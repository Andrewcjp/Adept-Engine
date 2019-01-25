#pragma once

#include "RHI/RHI_inc.h"

class GPUStateCache;
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
	void InsertStallTimerMarker();
	bool ShouldInsertTimer();
	void OnInsertStallTimer();
	PipelineStateObjectCache* GetPSOCache() const;
	void UpdatePSOTracker(RHIPipeLineStateObject* PSO);
	int GetNodeIndex();
	GPUStateCache* GetStateCache()const;
	bool IsDeviceNVIDIA();
	bool IsDeviceAMD();
	bool IsDeviceIntel();
protected:
	bool LogDeviceDebug = true;
	int DeviceIndex = 0;
	bool InsertStallTimer = false;
	//copy queue management 
	bool CopyEngineHasWork = false;
	RHICommandList* InterGPUCopyList = nullptr;
	RHICommandList* GPUCopyList = nullptr;
	CapabilityData Caps_Data = CapabilityData();
	PipelineStateObjectCache* PSOCache = nullptr;
	RHIPipeLineStateObject* CurrentGPUPSO = nullptr;
	//For creating a fake device in Linked adapter mode
	int NodeIndex = 0;
	GPUStateCache* StateCache = nullptr;
	uint VendorID = 0;
};

class RHIGPUSyncEvent
{
public:
	RHI_API RHIGPUSyncEvent(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue, DeviceContext * Device);
	RHI_API virtual ~RHIGPUSyncEvent();
	RHI_API virtual void Signal() = 0;
	RHI_API virtual void Wait() = 0;
	const char* GetDebugName()
	{
		return "GPUSyncEvent";
	}
protected:
	DeviceContext* Device = nullptr;
};
