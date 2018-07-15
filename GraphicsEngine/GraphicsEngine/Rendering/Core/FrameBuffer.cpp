#include "FrameBuffer.h"
#include "RHI/DeviceContext.h"
#include "../Core/Performance/PerfManager.h"
FrameBuffer::FrameBuffer(DeviceContext * device, RHIFrameBufferDesc & Desc)
{
	CurrentDevice = device;
	BufferDesc = Desc;
	m_width = BufferDesc.Width;
	m_height = BufferDesc.Height;
}

FrameBuffer::~FrameBuffer()
{
}

DeviceContext * FrameBuffer::GetDevice()
{
	return CurrentDevice;
}

RHIFrameBufferDesc & FrameBuffer::GetDescription() 
{
	return BufferDesc;
}

void FrameBuffer::Resize(int width, int height)
{}

void FrameBuffer::CopyHelper(FrameBuffer * Target, DeviceContext * TargetDevice)
{
	PerfManager::StartTimer("RunOnSecondDevice");
	DeviceContext* HostDevice = Target->GetDevice();
	if (TargetDevice == HostDevice)
	{
		return;
	}
	HostDevice->InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);
	RHICommandList* CopyList = HostDevice->GetInterGPUCopyList();
	CopyList->ResetList();
	CopyList->CopyResourceToSharedMemory(Target);
	CopyList->Execute(DeviceContextQueue::InterCopy);
	HostDevice->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	RHI::GetDeviceContext(1)->GPUWaitForOtherGPU(RHI::GetDeviceContext(0), DeviceContextQueue::Graphics, DeviceContextQueue::Graphics);
	TargetDevice->InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);

	CopyList = TargetDevice->GetInterGPUCopyList();
	CopyList->ResetList();
	CopyList->CopyResourceFromSharedMemory(Target);
	CopyList->Execute(DeviceContextQueue::InterCopy);
	TargetDevice->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	PerfManager::EndTimer("RunOnSecondDevice");
}

