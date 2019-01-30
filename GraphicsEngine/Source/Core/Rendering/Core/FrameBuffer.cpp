#include "FrameBuffer.h"
#include "RHI/DeviceContext.h"
#include "Core/Performance/PerfManager.h"
#include "RHI/RHI.h"
#include "RHI/SFRController.h"
#include "Core/Platform/PlatformCore.h"

FrameBuffer::FrameBuffer(DeviceContext * device, const RHIFrameBufferDesc & Desc)
{
	BufferDesc = Desc;
	const RHIFrameBufferDesc& T = Desc;
	BufferDesc = T;
	m_width = BufferDesc.Width;
	m_height = BufferDesc.Height;
	Device = device;
	HandleInit();
}

void FrameBuffer::HandleInit()
{
	BufferDesc.ViewPort = glm::vec4(0, 0, BufferDesc.Width, BufferDesc.Height);
	BufferDesc.ScissorRect = glm::vec4(0, 0, BufferDesc.Width, BufferDesc.Height);
	if (RHI::GetMGPUMode()->MainPassSFR && BufferDesc.IncludedInSFR)
	{
		SFR_Node = RHI::GetSplitController()->GetNode(Device->GetDeviceIndex());
		BufferDesc.ViewPort = glm::vec4(0, 0, BufferDesc.Width, BufferDesc.Height);
		const float start = BufferDesc.Width*SFR_Node->SFR_Offset;
		BufferDesc.Width = glm::iround(BufferDesc.Width*SFR_Node->SFR_PercentSize);
		BufferDesc.Height = glm::iround(BufferDesc.Height*SFR_Node->SFR_VerticalPercentSize);
		BufferDesc.ScissorRect = glm::ivec4(start, 0, start + BufferDesc.Width, BufferDesc.Height);
	}
}

FrameBuffer::~FrameBuffer()
{}

RHIFrameBufferDesc & FrameBuffer::GetDescription()
{
	return BufferDesc;
}

void FrameBuffer::Resize(int width, int height)
{
	BufferDesc.Width = width;
	BufferDesc.Height = height;
	HandleInit();
	HandleResize();
}

void FrameBuffer::HandleResize()
{

}

void FrameBuffer::CopyHelper(FrameBuffer * Target, DeviceContext * TargetDevice)
{
	PerfManager::StartTimer("RunOnSecondDevice");
	DeviceContext* HostDevice = Target->GetDevice();
	if (TargetDevice == HostDevice)
	{
		return;
	}
	HostDevice->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	HostDevice->InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);
	RHICommandList* CopyList = HostDevice->GetInterGPUCopyList();
	CopyList->ResetList();
	CopyList->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->CopyResourceToSharedMemory(Target);
	CopyList->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->ResolveTimers();
	CopyList->Execute(DeviceContextQueue::InterCopy);
	HostDevice->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	RHI::GetDeviceContext(1)->GPUWaitForOtherGPU(RHI::GetDeviceContext(0), DeviceContextQueue::InterCopy, DeviceContextQueue::InterCopy);

	//TargetDevice->InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);
	CopyList = TargetDevice->GetInterGPUCopyList();
	CopyList->ResetList();

	CopyList->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->CopyResourceFromSharedMemory(Target);
	CopyList->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->ResolveTimers();
	CopyList->Execute(DeviceContextQueue::InterCopy);
	TargetDevice->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	PerfManager::EndTimer("RunOnSecondDevice");
}

void FrameBuffer::BindDepthWithColourPassthrough(RHICommandList * list, FrameBuffer * PassThrough)
{}

void FrameBuffer::ResolveSFR()
{
}

void FrameBuffer::CopyHelper_Async(FrameBuffer * Target, DeviceContext * TargetDevice)
{
	PerfManager::StartTimer("RunOnSecondDevice");
	DeviceContext* HostDevice = Target->GetDevice();
	if (TargetDevice == HostDevice)
	{
		return;
	}
	HostDevice->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	HostDevice->InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);
	RHICommandList* CopyList = HostDevice->GetInterGPUCopyList();
	CopyList->ResetList();
	CopyList->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->CopyResourceToSharedMemory(Target);
	CopyList->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->ResolveTimers();
	CopyList->Execute(DeviceContextQueue::InterCopy);
	HostDevice->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	RHI::GetDeviceContext(1)->GPUWaitForOtherGPU(RHI::GetDeviceContext(0), DeviceContextQueue::InterCopy, DeviceContextQueue::InterCopy);

	//TargetDevice->InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);
	CopyList = TargetDevice->GetInterGPUCopyList();
	CopyList->ResetList();
	CopyList->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->CopyResourceFromSharedMemory(Target);
	CopyList->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->ResolveTimers();
	CopyList->Execute(DeviceContextQueue::InterCopy);
	TargetDevice->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	PerfManager::EndTimer("RunOnSecondDevice");
}

void FrameBuffer::CopyHelper_Async_OneFrame(FrameBuffer * Target, DeviceContext * TargetDevice)
{
	//PerfManager::StartTimer("RunOnSecondDevice");
	DeviceContext* HostDevice = Target->GetDevice();
	if (TargetDevice == HostDevice)
	{
		return;
	}
	RHICommandList* CopyList = nullptr;
#if 1

	HostDevice->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	HostDevice->InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);
	CopyList = HostDevice->GetInterGPUCopyList();
	CopyList->ResetList();
	CopyList->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->CopyResourceToSharedMemory(Target);
	CopyList->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->ResolveTimers();
	CopyList->Execute(DeviceContextQueue::InterCopy);
	HostDevice->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	RHI::GetDeviceContext(1)->GPUWaitForOtherGPU(RHI::GetDeviceContext(0), DeviceContextQueue::InterCopy, DeviceContextQueue::InterCopy);
#endif
	//TargetDevice->InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);
	CopyList = TargetDevice->GetInterGPUCopyList();
	CopyList->ResetList();

	CopyList->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->CopyResourceFromSharedMemory(Target);
	CopyList->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->ResolveTimers();
	CopyList->Execute(DeviceContextQueue::InterCopy);
	TargetDevice->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	//PerfManager::EndTimer("RunOnSecondDevice");
}