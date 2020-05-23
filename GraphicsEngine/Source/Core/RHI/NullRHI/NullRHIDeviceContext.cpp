#include "Stdafx.h"
#include "NullRHIDeviceContext.h"
#include "RHI/RHI.h"

void NullRHIDeviceContext::ResetDeviceAtEndOfFrame()
{

}

void NullRHIDeviceContext::SampleVideoMemoryInfo()
{

}

RHIClass::GPUMemoryData NullRHIDeviceContext::GetMemoryReport()
{
	return RHIClass::GPUMemoryData();
}

void NullRHIDeviceContext::DestoryDevice()
{

}

void NullRHIDeviceContext::WaitForGpu()
{

}

void NullRHIDeviceContext::WaitForCopy()
{

}

void NullRHIDeviceContext::NotifyWorkForCopyEngine()
{

}

void NullRHIDeviceContext::UpdateCopyEngine()
{

}

void NullRHIDeviceContext::ResetCopyEngine()
{

}

void NullRHIDeviceContext::GPUWaitForOtherGPU(DeviceContext* OtherGPU, DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue)
{

}

void NullRHIDeviceContext::CPUWaitForAll()
{

}

void NullRHIDeviceContext::InsertGPUWait(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue)
{

}

void NullRHIDeviceContext::InsertCrossGPUWait(DeviceContextQueue::Type WaitingQueue, DeviceContext* SignalingGPU, DeviceContextQueue::Type SignalQueue)
{

}

RHITimeManager* NullRHIDeviceContext::GetTimeManager()
{
	return nullptr;
}

bool NullRHIDeviceContext::IsUMA() const
{
	return false;
}
