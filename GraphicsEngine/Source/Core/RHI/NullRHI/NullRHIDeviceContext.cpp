#include "NullRHIDeviceContext.h"
#include "RHI/RHI.h"

NullRHIDeviceContext::NullRHIDeviceContext()
{
	Caps_Data.SupportTypedUAVLoads = true;
	Caps_Data.RTSupport = ERayTracingSupportType::Hardware;
	Caps_Data.VRSSupport = EVRSSupportType::Hardware_Tier2;
	Caps_Data.VRSTileSize = 16;
}

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
