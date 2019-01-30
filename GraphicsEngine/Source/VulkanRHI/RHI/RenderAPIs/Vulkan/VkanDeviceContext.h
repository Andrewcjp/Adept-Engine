#pragma once
#include "RHI/DeviceContext.h"
class VkanDeviceContext:public DeviceContext
{
public:
	VkanDeviceContext();
	~VkanDeviceContext();

	// Inherited via DeviceContext
	virtual void ResetDeviceAtEndOfFrame() override;
	virtual void SampleVideoMemoryInfo() override;
	virtual std::string GetMemoryReport() override;
	virtual void DestoryDevice() override;
	virtual void WaitForGpu() override;
	virtual void WaitForCopy() override;
	virtual void ResetSharingCopyList() override;
	virtual void NotifyWorkForCopyEngine() override;
	virtual void UpdateCopyEngine() override;
	virtual void ResetCopyEngine() override;
	virtual int GetDeviceIndex() override;
	virtual int GetCpuFrameIndex() override;
	virtual void GPUWaitForOtherGPU(DeviceContext * OtherGPU, DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue) override;
	virtual void CPUWaitForAll() override;
	virtual void InsertGPUWait(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue) override;
	virtual RHITimeManager * GetTimeManager() override;
private:
	class VkanTimeManager* TimeManager = nullptr;
};

