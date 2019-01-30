#include "VkanDeviceContext.h"
#include "VkanTimeManager.h"


VkanDeviceContext::VkanDeviceContext()
{
	TimeManager = new VkanTimeManager();
}


VkanDeviceContext::~VkanDeviceContext()
{}

void VkanDeviceContext::ResetDeviceAtEndOfFrame()
{}

void VkanDeviceContext::SampleVideoMemoryInfo()
{}

std::string VkanDeviceContext::GetMemoryReport()
{
	return std::string();
}

void VkanDeviceContext::DestoryDevice()
{}

void VkanDeviceContext::WaitForGpu()
{}

void VkanDeviceContext::WaitForCopy()
{}

void VkanDeviceContext::ResetSharingCopyList()
{}

void VkanDeviceContext::NotifyWorkForCopyEngine()
{}

void VkanDeviceContext::UpdateCopyEngine()
{}

void VkanDeviceContext::ResetCopyEngine()
{}

int VkanDeviceContext::GetDeviceIndex()
{
	return 0;
}

int VkanDeviceContext::GetCpuFrameIndex()
{
	return 0;
}

void VkanDeviceContext::GPUWaitForOtherGPU(DeviceContext * OtherGPU, DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue)
{}

void VkanDeviceContext::CPUWaitForAll()
{}

void VkanDeviceContext::InsertGPUWait(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue)
{}

RHITimeManager * VkanDeviceContext::GetTimeManager()
{
	return TimeManager;
}
