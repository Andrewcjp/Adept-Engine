#include "DeviceContext.h"
#include "Core/Asserts.h"
#include "RHITypes.h"
#include "RHITimeManager.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Platform/PlatformCore.h"
#include "Rendering/Renderers/TextRenderer.h"

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
	CopyListPoolFreeIndex = 0;//reset the copy pool indexer
}

void DeviceContext::DestoryDevice()
{
	for (int i = 0; i < COPYLIST_MAX_POOL_SIZE; i++)
	{
		EnqueueSafeRHIRelease(CopyListPool[i]);
	}
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
	if (Index < 0 || Index > COPYLIST_MAX_POOL_SIZE)
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

void DeviceContext::TickTransferStats()
{
	GetTransferBytes();
	if (BytesToTransfer == 0)
	{
		return;
	}
	PerfManager::TimerData* t = PerfManager::Get()->GetTimerData(PerfManager::Get()->GetTimerIDByName("MGPU Copy" + std::to_string(GetDeviceIndex())));
	if (t == nullptr)
	{
		return;
	}
	//SFR
	float transferTimeInS = t->Time / 10e3;
	float MB = (float)BytesToTransfer / 10e6;
	float TransferSpeedBPerS = (float)BytesToTransfer / transferTimeInS;//1024
	std::stringstream ss;
	ss << "GPU_" << GetDeviceIndex() << " transferring " << std::fixed << std::setprecision(2) << MB << "Mb @ " << TransferSpeedBPerS / 10e9 << "GB/s Taking " << t->Time << "ms";
	TextRenderer::instance->RenderFromAtlas(ss.str(), 100, 20 + 20 * GetDeviceIndex(), 0.35f);
}
int DeviceContext::GetTransferBytes()
{
	BytesToTransfer = 0;
	for (int i = 0; i < BuffersWithTransfers.size(); i++)
	{
		BytesToTransfer += BuffersWithTransfers[i]->GetTransferSize();
	}
	return BytesToTransfer;
}

void DeviceContext::AddTransferBuffer(FrameBuffer * buffer)
{
	for (int i = 0; i < BuffersWithTransfers.size(); i++)
	{
		if (BuffersWithTransfers[i] == buffer)
		{
			return;
		}
	}
	BuffersWithTransfers.push_back(buffer);
}

void DeviceContext::RemoveTransferBuffer(FrameBuffer * buffer)
{
	VectorUtils::Remove(BuffersWithTransfers, buffer);
}

void DeviceContext::ResetStat()
{
	for (int i = 0; i < BuffersWithTransfers.size(); i++)
	{
		BuffersWithTransfers[i]->ResetTransferStat();
	}
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
