#include "DeviceContext.h"
#include "Core/Asserts.h"
#include "RHITypes.h"
#include "RHITimeManager.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Platform/PlatformCore.h"

DeviceContext::DeviceContext()
{
	PSOCache = new PipelineStateObjectCache(this);
	StateCache = new GPUStateCache();
}

DeviceContext::~DeviceContext()
{
	SafeDelete(PSOCache);
}


void DeviceContext::ResetDeviceAtEndOfFrame()
{
	CopyListPoolFreeIndex = 0;
	if (CurrentFrameIndex == 0)
	{
		GetTimeManager()->UpdateTimers();
	}
}

void DeviceContext::DestoryDevice()
{

}

const CapabilityData & DeviceContext::GetCaps()
{
	return Caps_Data;
}

RHICommandList * DeviceContext::GetInterGPUCopyList()
{
	return InterGPUCopyList;
}

void DeviceContext::InsertStallTimerMarker()
{
	InsertStallTimer = true;
}

bool DeviceContext::ShouldInsertTimer()
{
	return InsertStallTimer;
}

void DeviceContext::OnInsertStallTimer()
{
	InsertStallTimer = false;
}

PipelineStateObjectCache * DeviceContext::GetPSOCache() const
{
	return PSOCache;
}

void DeviceContext::UpdatePSOTracker(RHIPipeLineStateObject * PSO)
{
	if (CurrentGPUPSO != PSO)
	{
		SCOPE_CYCLE_COUNTER_GROUP("PSO switches", "RHI");
	}
	CurrentGPUPSO = PSO;
}

int DeviceContext::GetNodeIndex()
{
	return NodeIndex;
}

GPUStateCache * DeviceContext::GetStateCache() const
{
	return StateCache;
}

bool DeviceContext::IsDeviceNVIDIA()
{
	return VendorID == 0x10DE;
}

bool DeviceContext::IsDeviceAMD()
{
	return VendorID = 0x1002;
}

bool DeviceContext::IsDeviceIntel()
{
	return VendorID == 0x8086;
}

RHICommandList * DeviceContext::GetCopyList(int Index)
{
	if (Index < 0 || Index > COPYLIST_POOL_SIZE)
	{
		ensure(false);
		return nullptr;
	}
	if (CopyListPool[Index] == nullptr)
	{
		CopyListPool[Index] = RHI::CreateCommandList(ECommandListType::Copy, this);
	}
	return CopyListPool[Index];
}

RHICommandList * DeviceContext::GetNextFreeCopyList()
{
	RHICommandList* retvalue = GetCopyList(CopyListPoolFreeIndex);
	CopyListPoolFreeIndex++;
	return retvalue;
}

void DeviceContext::InitCopyListPool()
{
	for (int i = 0; i < RHI::GetRenderConstants()->DEFAULT_COPYLIST_POOL_SIZE; i++)
	{
		GetCopyList(i);
	}
}

RHIGPUSyncEvent::RHIGPUSyncEvent(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue, DeviceContext * device)
{
	Device = device;
}

RHIGPUSyncEvent::~RHIGPUSyncEvent()
{

}
