#include "FrameBuffer.h"
#include "RHI/DeviceContext.h"
#include "Core/Performance/PerfManager.h"
#include "RHI/RHI.h"
#include "RHI/SFRController.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/BaseWindow.h"

#include "Screen.h"
#include "RHI/RHITexture.h"
#include "RHI/RHICommandList.h"

FrameBuffer::FrameBuffer(DeviceContext * device, const RHIFrameBufferDesc & Desc)
{
	BufferDesc = Desc;
	const RHIFrameBufferDesc& T = Desc;
	BufferDesc = T;
	Device = device;
	m_width = BufferDesc.Width;
	m_height = BufferDesc.Height;
	HandleInit();
	SetupFences();
}

void FrameBuffer::HandleInit()
{
	BufferDesc.ViewPort = glm::vec4(0, 0, BufferDesc.Width, BufferDesc.Height);
	BufferDesc.ScissorRect = glm::vec4(0, 0, BufferDesc.Width, BufferDesc.Height);
	BufferDesc.SFR_FullWidth = BufferDesc.Width;
	if (/*RHI::GetMGPUSettings()->MainPassSFR && */BufferDesc.IncludedInSFR)
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
}

void FrameBuffer::PostInit()
{
	if (BufferDesc.AllowUnorderedAccess)
	{
	}
}

FrameBuffer::~FrameBuffer()
{}

int FrameBuffer::GetWidth() const
{
	return m_width;
}

int FrameBuffer::GetHeight() const
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
	PostInit();
}

void FrameBuffer::SFRResize()
{
	Resize(m_width, m_height);
}


void FrameBuffer::HandleResize()
{

}

void FrameBuffer::SetupFences()
{
	//CopyFence = RHI::CreateSyncEvent(DeviceContextQueue::Copy, DeviceContextQueue::Copy);
	//DeviceFence = RHI::CreateSyncEvent(DeviceContextQueue::Copy, DeviceContextQueue::Copy, RHI::GetDeviceContext(0), RHI::GetDeviceContext(1));
	//TargetCopyFence = RHI::CreateSyncEvent(DeviceContextQueue::Copy, DeviceContextQueue::Copy, RHI::GetDeviceContext(0), RHI::GetDeviceContext(0));
}

void FrameBuffer::CopyHelper_NewSync(FrameBuffer * Target, DeviceContext * TargetDevice, EGPUCOPYTIMERS::Type Stat, EDeviceContextQueue::Type CopyQ/* = DeviceContextQueue::Copy*/)
{
	PerfManager::StartTimer("RunOnSecondDevice");
	DeviceContext* HostDevice = Target->GetDevice();
	if (TargetDevice == HostDevice)
	{
		return;
	}
	CopyQ = EDeviceContextQueue::InterCopy;
	HostDevice->InsertGPUWait(EDeviceContextQueue::Graphics, CopyQ);
	HostDevice->InsertGPUWait(CopyQ, EDeviceContextQueue::Graphics);
	RHICommandList* CopyList = HostDevice->GetNextFreeCopyList();
	//Target->CopyFence->Wait();
	CopyList->ResetList();
	CopyList->StartTimer(Stat);
	CopyList->CopyResourceToSharedMemory(Target);
	CopyList->EndTimer(Stat);
	CopyList->ResolveTimers();
	CopyList->Execute(CopyQ);
	//Target->CopyFence->Signal();
	//if (!RHI::GetMGPUSettings()->AsyncShadows)
	{
		HostDevice->InsertGPUWait(EDeviceContextQueue::Graphics, CopyQ);
	}
	//RHI::GetDeviceContext(1)->GPUWaitForOtherGPU(RHI::GetDeviceContext(0), CopyQ, CopyQ);
	//if (!RHI::GetMGPUSettings()->SFRSplitShadows)
	{
		HostDevice->GPUWaitForOtherGPU(TargetDevice, CopyQ, CopyQ);
	}
	TargetDevice->InsertGPUWait(CopyQ, EDeviceContextQueue::Graphics);
	CopyList = TargetDevice->GetNextFreeCopyList();
	CopyList->ResetList();
	CopyList->StartTimer(Stat);
	CopyList->CopyResourceFromSharedMemory(Target);
	CopyList->EndTimer(Stat);
	CopyList->ResolveTimers();
	CopyList->Execute(CopyQ);
	//TargetDevice->InsertGPUWait(DeviceContextQueue::Graphics, CopyQ);
	PerfManager::EndTimer("RunOnSecondDevice");
	//Multi shadow are missing a sync!
}


void FrameBuffer::CopyToStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* List)
{
	BufferDesc.RenderTargets[0]->CopyToStagingResource(Res, List);
}

void FrameBuffer::CopyFromStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* list)
{
	BufferDesc.RenderTargets[0]->CopyFromStagingResource(Res, list);
}

void FrameBuffer::SetResourceState(RHICommandList* List, EResourceState::Type State, bool ChangeDepth /*= false*/, EResourceTransitionMode::Type TransitionMode /*= EResourceTransitionMode::Direct*/)
{}

void FrameBuffer::Release()
{

}

size_t FrameBuffer::GetSizeOnGPU()
{
	size_t Size = 0;
	for (int i = 0; i < MRT_MAX; i++)
	{
		if (BufferDesc.RTFormats[i] != FORMAT_UNKNOWN)
		{
			Size += BufferDesc.Width*BufferDesc.Height*RHIUtils::GetPixelSize(BufferDesc.RTFormats[i]);
		}
	}
	if (BufferDesc.NeedsDepthStencil)
	{
		Size += BufferDesc.Width*BufferDesc.Height*RHIUtils::GetPixelSize(BufferDesc.DepthFormat);
	}
	return Size;
}


void FrameBuffer::AutoUpdateSize(RHIFrameBufferDesc& BufferDesc, DeviceContext* Device)
{
	if (BufferDesc.SizeMode == EFrameBufferSizeMode::Fixed)
	{
		Log::LogMessage("AutoResize Called on fixed size buffer this is invalid", Log::Error);
		return;
	}
	ensure(BufferDesc.LinkToBackBufferScaleFactor > 0.0f);
	if (BufferDesc.SizeMode == EFrameBufferSizeMode::LinkedToRenderScale)
	{
		BufferDesc.Height = glm::iround(Screen::GetScaledHeight()*BufferDesc.LinkToBackBufferScaleFactor);
		BufferDesc.Width = glm::iround(Screen::GetScaledWidth()*BufferDesc.LinkToBackBufferScaleFactor);
	}
	else if (BufferDesc.SizeMode == EFrameBufferSizeMode::LinkedToScreenSize)
	{
		BufferDesc.Height = glm::iround(Screen::GetWindowHeight()*BufferDesc.LinkToBackBufferScaleFactor);
		BufferDesc.Width = glm::iround(Screen::GetWindowWidth()*BufferDesc.LinkToBackBufferScaleFactor);
	}
	else if (BufferDesc.SizeMode == EFrameBufferSizeMode::LinkedToRenderScale_TileSize)
	{
		uint TileSize = BufferDesc.LinkToBackBufferScaleFactor;
		if (Device != nullptr)
		{
			TileSize = RHI::GetRenderSettings()->GetVRXSettings().VRSTileSize;
		}
		BufferDesc.Height = glm::iround(glm::ceil(Screen::GetScaledHeight() / TileSize));
		BufferDesc.Width = glm::iround(glm::ceil(Screen::GetScaledWidth() / TileSize));
	}
}

void FrameBuffer::AutoResize()
{
	if (BufferDesc.SizeMode == EFrameBufferSizeMode::Fixed)
	{
		Log::LogMessage("AutoResize Called on fixed size buffer this is invalid", Log::Error);
		return;
	}
	AutoUpdateSize(BufferDesc, Device);
	Resize(BufferDesc.Width, BufferDesc.Height);
}

void FrameBuffer::CopyHelper(FrameBuffer * Target, DeviceContext * TargetDevice, EGPUCOPYTIMERS::Type Stat, EDeviceContextQueue::Type CopyQ/* = DeviceContextQueue::Copy*/)
{

	PerfManager::StartTimer("RunOnSecondDevice");
	DeviceContext* HostDevice = Target->GetDevice();
	if (TargetDevice == HostDevice)
	{
		return;
	}
	CopyQ = EDeviceContextQueue::InterCopy;
	HostDevice->InsertGPUWait(EDeviceContextQueue::Graphics, CopyQ);
	HostDevice->InsertGPUWait(CopyQ, EDeviceContextQueue::Graphics);
	RHICommandList* CopyList = HostDevice->GetNextFreeCopyList();
	CopyList->ResetList();
	CopyList->StartTimer(Stat);
	CopyList->CopyResourceToSharedMemory(Target);
	CopyList->EndTimer(Stat);
	CopyList->ResolveTimers();
	CopyList->Execute(CopyQ);
	//	if (!RHI::GetMGPUSettings()->AsyncShadows)
	{
		HostDevice->InsertGPUWait(EDeviceContextQueue::Graphics, CopyQ);
	}
	//RHI::GetDeviceContext(1)->GPUWaitForOtherGPU(RHI::GetDeviceContext(0), CopyQ, CopyQ);

	HostDevice->GPUWaitForOtherGPU(TargetDevice, CopyQ, CopyQ);

	TargetDevice->InsertGPUWait(CopyQ, EDeviceContextQueue::Graphics);
	CopyList = TargetDevice->GetNextFreeCopyList();
	CopyList->ResetList();
	CopyList->StartTimer(Stat);
	CopyList->CopyResourceFromSharedMemory(Target);
	CopyList->EndTimer(Stat);
	CopyList->ResolveTimers();
	CopyList->Execute(CopyQ);
	//	if (RHI::GetMGPUSettings()->SFRSplitShadows)
	{
		//if (Stat != EGPUCOPYTIMERS::ShadowCopy)
		{
			TargetDevice->InsertGPUWait(EDeviceContextQueue::Graphics, CopyQ);
		}
	}
	//	else
	{
		TargetDevice->InsertGPUWait(EDeviceContextQueue::Graphics, CopyQ);
	}
	PerfManager::EndTimer("RunOnSecondDevice");
	//Multi shadow are missing a sync!
}

void FrameBuffer::CopyToOtherBuffer(FrameBuffer * OtherBuffer, RHICommandList* List)
{

}

void FrameBuffer::BindDepthWithColourPassthrough(RHICommandList * list, FrameBuffer * PassThrough)
{}

void FrameBuffer::MakeReadyForComputeUse(RHICommandList * List, bool Depth)
{
	SetResourceState(List, EResourceState::ComputeUse, Depth);
}

void FrameBuffer::MakeReadyForCopy(RHICommandList * list, bool changeDepth /*= false*/)
{
	SetResourceState(list, EResourceState::CopySrc, changeDepth);
}

void FrameBuffer::MakeReadyForPixel(RHICommandList * List, bool Depth)
{
	SetResourceState(List, EResourceState::PixelShader, Depth);
}

EResourceState::Type FrameBuffer::GetCurrentState() const
{
	return CurrentState;
}

uint64 FrameBuffer::GetInstanceHash() const
{
	return uint64();
}

RHITexture * FrameBuffer::GetRenderTexture(int index)
{
	return BufferDesc.RenderTargets[index];
}

RHITexture * FrameBuffer::GetDepthStencil()
{
	return BufferDesc.DepthStencil;
}

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
	//	ensure(Target->GetDescription().IsShared);
	HostDevice->InsertGPUWait(EDeviceContextQueue::Graphics, EDeviceContextQueue::InterCopy);
	HostDevice->InsertGPUWait(EDeviceContextQueue::InterCopy, EDeviceContextQueue::Graphics);
	RHICommandList* CopyList = HostDevice->GetInterGPUCopyList();
	CopyList->ResetList();
	CopyList->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->CopyResourceToSharedMemory(Target);
	CopyList->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->ResolveTimers();
	CopyList->Execute(EDeviceContextQueue::InterCopy);
	HostDevice->InsertGPUWait(EDeviceContextQueue::Graphics, EDeviceContextQueue::InterCopy);
	HostDevice->GPUWaitForOtherGPU(TargetDevice, EDeviceContextQueue::InterCopy, EDeviceContextQueue::InterCopy);
	//use a sync point here?
#if 1//_DEBUG
	TargetDevice->InsertGPUWait(EDeviceContextQueue::InterCopy, EDeviceContextQueue::Graphics);//the CPU is ahead of the gpu so resource is still in use! (ithink)
#endif
	CopyList = TargetDevice->GetInterGPUCopyList();
	CopyList->ResetList();

	CopyList->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->CopyResourceFromSharedMemory(Target);
	CopyList->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
#if 0
	CopyList->StartTimer(EGPUCOPYTIMERS::SFRMerge);
	Target->CopyToOtherBuffer(SumBuffer, CopyList);
	CopyList->EndTimer(EGPUCOPYTIMERS::SFRMerge);
#endif
	CopyList->Execute(EDeviceContextQueue::InterCopy);

	TargetDevice->InsertGPUWait(EDeviceContextQueue::InterCopy, EDeviceContextQueue::Graphics);
	TargetDevice->InsertGPUWait(EDeviceContextQueue::Graphics, EDeviceContextQueue::InterCopy);
#if 1
	CopyList = TargetDevice->GetNextFreeCopyList();
	CopyList->ResetList();
	CopyList->StartTimer(EGPUCOPYTIMERS::SFRMerge);
	Target->CopyToOtherBuffer(SumBuffer, CopyList);
	CopyList->EndTimer(EGPUCOPYTIMERS::SFRMerge);
	CopyList->ResolveTimers();
	CopyList->Execute(EDeviceContextQueue::InterCopy);
#endif
	TargetDevice->InsertGPUWait(EDeviceContextQueue::Graphics, EDeviceContextQueue::InterCopy);
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
	return BufferDesc.IncludedInSFR /*&& RHI::GetMGPUSettings()->MainPassSFR*/;
}

void FrameBuffer::CopyHelper_Async(FrameBuffer * Target, DeviceContext * TargetDevice)
{
	PerfManager::StartTimer("RunOnSecondDevice");
	DeviceContext* HostDevice = Target->GetDevice();
	if (TargetDevice == HostDevice)
	{
		return;
	}
	HostDevice->InsertGPUWait(EDeviceContextQueue::Graphics, EDeviceContextQueue::InterCopy);
	HostDevice->InsertGPUWait(EDeviceContextQueue::InterCopy, EDeviceContextQueue::Graphics);
	RHICommandList* CopyList = HostDevice->GetInterGPUCopyList();
	CopyList->ResetList();
	CopyList->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->CopyResourceToSharedMemory(Target);
	CopyList->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->ResolveTimers();
	CopyList->Execute(EDeviceContextQueue::InterCopy);
	HostDevice->InsertGPUWait(EDeviceContextQueue::Graphics, EDeviceContextQueue::InterCopy);
	RHI::GetDeviceContext(1)->GPUWaitForOtherGPU(RHI::GetDeviceContext(0), EDeviceContextQueue::InterCopy, EDeviceContextQueue::InterCopy);

	TargetDevice->InsertGPUWait(EDeviceContextQueue::InterCopy, EDeviceContextQueue::Graphics);
	CopyList = TargetDevice->GetInterGPUCopyList();
	CopyList->ResetList();
	CopyList->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->CopyResourceFromSharedMemory(Target);
	CopyList->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->ResolveTimers();
	CopyList->Execute(EDeviceContextQueue::InterCopy);
	TargetDevice->InsertGPUWait(EDeviceContextQueue::Graphics, EDeviceContextQueue::InterCopy);
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

	HostDevice->InsertGPUWait(EDeviceContextQueue::Graphics, EDeviceContextQueue::InterCopy);
	HostDevice->InsertGPUWait(EDeviceContextQueue::InterCopy, EDeviceContextQueue::Graphics);
	CopyList = HostDevice->GetInterGPUCopyList();
	CopyList->ResetList();
	CopyList->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->CopyResourceToSharedMemory(Target);
	CopyList->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->ResolveTimers();
	CopyList->Execute(EDeviceContextQueue::InterCopy);
	HostDevice->InsertGPUWait(EDeviceContextQueue::Graphics, EDeviceContextQueue::InterCopy);
	RHI::GetDeviceContext(1)->GPUWaitForOtherGPU(RHI::GetDeviceContext(0), EDeviceContextQueue::InterCopy, EDeviceContextQueue::InterCopy);
#endif
	//TargetDevice->InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);
	CopyList = TargetDevice->GetInterGPUCopyList();
	CopyList->ResetList();

	CopyList->StartTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->CopyResourceFromSharedMemory(Target);
	CopyList->EndTimer(EGPUCOPYTIMERS::MGPUCopy);
	CopyList->ResolveTimers();
	CopyList->Execute(EDeviceContextQueue::InterCopy);
	TargetDevice->InsertGPUWait(EDeviceContextQueue::Graphics, EDeviceContextQueue::InterCopy);
	//PerfManager::EndTimer("RunOnSecondDevice");
}