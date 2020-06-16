#pragma once
#include "RHI/DeviceContext.h"
#include "RHI/RHI.h"
class NullRHIDeviceContext : public DeviceContext
{

public:
	NullRHIDeviceContext();
	void ResetDeviceAtEndOfFrame() override;
	void SampleVideoMemoryInfo() override;
	RHIClass::GPUMemoryData GetMemoryReport() override;
	void DestoryDevice() override;
	void WaitForGpu() override;
	void WaitForCopy() override;
	void NotifyWorkForCopyEngine() override;
	void UpdateCopyEngine() override;
	void ResetCopyEngine() override;
	void GPUWaitForOtherGPU(DeviceContext* OtherGPU, EDeviceContextQueue::Type WaitingQueue, EDeviceContextQueue::Type SignalQueue) override;
	void CPUWaitForAll() override;
	void InsertGPUWait(EDeviceContextQueue::Type WaitingQueue, EDeviceContextQueue::Type SignalQueue) override;
	void InsertCrossGPUWait(EDeviceContextQueue::Type WaitingQueue, DeviceContext* SignalingGPU, EDeviceContextQueue::Type SignalQueue) override;
	class RHITimeManager* GetTimeManager() override;
	bool IsUMA() const override;
};

