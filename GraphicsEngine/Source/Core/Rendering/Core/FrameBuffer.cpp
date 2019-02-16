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
	Device = device;
	m_width = BufferDesc.Width;
	m_height = BufferDesc.Height;
	HandleInit();
}

void FrameBuffer::HandleInit()
{
	BufferDesc.ViewPort = glm::vec4(0, 0, BufferDesc.Width, BufferDesc.Height);
	BufferDesc.ScissorRect = glm::vec4(0, 0, BufferDesc.Width, BufferDesc.Height);
	BufferDesc.SFR_FullWidth = BufferDesc.Width;
	if (RHI::GetMGPUSettings()->MainPassSFR && BufferDesc.IncludedInSFR)
	{		
		SFR_Node = RHI::GetSplitController()->GetNode(Device->GetDeviceIndex());
		SFR_Node->AddBuffer(this);
		BufferDesc.ViewPort = glm::vec4(0, 0, BufferDesc.Width, BufferDesc.Height);
		const float start = BufferDesc.Width*SFR_Node->SFR_Offset;//Offset is in whole buffer space
		const int SFrBufferWidth = glm::iround(BufferDesc.Width*SFR_Node->SFR_PercentSize);
		const int SFrBufferHeight = glm::iround(BufferDesc.Height*SFR_Node->SFR_VerticalPercentSize);
		if (Device->GetDeviceIndex() > 0)
		{
			Log::LogMessage("Sfr Buffer Updated to " + std::to_string(SFrBufferWidth) + "X" + std::to_string(SFrBufferHeight));
		}		
		BufferDesc.ScissorRect = glm::ivec4(start, 0, start + SFrBufferWidth, SFrBufferHeight);
	}
	if (BufferDesc.DeviceToCopyTo != nullptr)
	{
		Device->AddTransferBuffer(this);
		BufferDesc.DeviceToCopyTo->AddTransferBuffer(this);
	}
}

FrameBuffer::~FrameBuffer()
{}

inline int FrameBuffer::GetWidth() const
{
	return m_width;
}

inline int FrameBuffer::GetHeight() const
{
	return m_height;
}

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

void FrameBuffer::SFRResize()
{
	Resize(m_width, m_height);
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
	RHICommandList* CopyList = HostDevice->GetNextFreeCopyList();
	CopyList->ResetList();
	CopyList->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->CopyResourceToSharedMemory(Target);
	CopyList->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->ResolveTimers();
	CopyList->Execute(DeviceContextQueue::InterCopy);
	HostDevice->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	RHI::GetDeviceContext(1)->GPUWaitForOtherGPU(RHI::GetDeviceContext(0), DeviceContextQueue::InterCopy, DeviceContextQueue::InterCopy);

	TargetDevice->InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);
	CopyList = TargetDevice->GetNextFreeCopyList();
	CopyList->ResetList();

	CopyList->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->CopyResourceFromSharedMemory(Target);
	CopyList->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->ResolveTimers();
	CopyList->Execute(DeviceContextQueue::InterCopy);
	TargetDevice->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	PerfManager::EndTimer("RunOnSecondDevice");
}

void FrameBuffer::CopyToOtherBuffer(FrameBuffer * OtherBuffer, RHICommandList* List)
{

}

void FrameBuffer::BindDepthWithColourPassthrough(RHICommandList * list, FrameBuffer * PassThrough)
{}

void FrameBuffer::ResolveSFR(FrameBuffer* SumBuffer)
{
	if (!NeedsSFRResolve())
	{
		return;
	}
	DeviceContext* TargetDevice = RHI::GetDeviceContext(0);
	FrameBuffer* Target = this;
	DeviceContext* HostDevice = Target->GetDevice();
	if (TargetDevice == HostDevice)
	{
		return;
	}
	ensure(Target->GetDescription().IsShared);
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
	//use a sync point here?
#if 1//_DEBUG
	TargetDevice->InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);//the CPU is ahead of the gpu so resource is still in use! (ithink)
#endif
	CopyList = TargetDevice->GetInterGPUCopyList();
	CopyList->ResetList();

	CopyList->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->CopyResourceFromSharedMemory(Target);
	CopyList->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->Execute(DeviceContextQueue::InterCopy);

	TargetDevice->InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);
	//TargetDevice->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	CopyList = TargetDevice->GetCopyList(1);
	CopyList->ResetList();
	CopyList->StartTimer(EGPUCOPYTIMERS::SFRMerge);
	Target->CopyToOtherBuffer(SumBuffer, CopyList);
	CopyList->EndTimer(EGPUCOPYTIMERS::SFRMerge);
	CopyList->ResolveTimers();
	CopyList->Execute(DeviceContextQueue::InterCopy);
	TargetDevice->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	//Copy to display device collect all devices
	//reset the Viewport to be correct
}

int FrameBuffer::GetTransferSize()
{
	if (!DidTransferLastFrame)
	{
		return 0;
	}
	return CrossGPUBytes;
}

void FrameBuffer::ResetTransferStat()
{
	DidTransferLastFrame = false;
}

bool FrameBuffer::NeedsSFRResolve() const
{
	return BufferDesc.IncludedInSFR && RHI::GetMGPUSettings()->MainPassSFR;
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

	TargetDevice->InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);
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