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

void DeviceContext::TickTransferStats(bool render)
{
	GetTransferBytes();
	if (BytesToTransfer == 0)
	{
		return;
	}
	float TotalTime = 0;
	TimerData* t = PerfManager::Get()->GetTimerData(PerfManager::Get()->GetTimerIDByName("MGPU Copy" + std::to_string(GetDeviceIndex())));
	if (t != nullptr)
	{
		TotalTime += t->Time;
	}
	t = PerfManager::Get()->GetTimerData(PerfManager::Get()->GetTimerIDByName("Shadow Copy" + std::to_string(GetDeviceIndex())));
	if (t != nullptr)
	{
		TotalTime += t->Time;
	}
	t = PerfManager::Get()->GetTimerData(PerfManager::Get()->GetTimerIDByName("2Shadow Copy2" + std::to_string(GetDeviceIndex())));
	if (t != nullptr)
	{
		TotalTime += t->Time;
	}

	//SFR
	float transferTimeInS = TotalTime / 10e3f;
	float MB = (float)BytesToTransfer / 10e6f;
	float TransferSpeedBPerS = (float)BytesToTransfer / transferTimeInS;//1024
	PerfManager::Get()->UpdateStat(PerfManager::Get()->GetTimerIDByName("TransferBytes" + std::to_string(GetDeviceIndex())), MB, 0);
	if (render)
	{
		std::stringstream ss;
		ss << "GPU_" << GetDeviceIndex() << " transferring " << std::fixed << std::setprecision(2) << MB << "Mb @ " << TransferSpeedBPerS / 10e9 << "GB/s Taking " << TotalTime << "ms";
		TextRenderer::instance->RenderFromAtlas(ss.str(), 100.0f, 20.0f + 20 * GetDeviceIndex(), 0.35f);
	}
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

uint DeviceContext::GetNodeMask()
{
	return GPUMask;
}
//#SLI Create struct for GPU mask settings
void DeviceContext::SetNodeMaskFromIndex(int GPUIndex)
{
	GPUMask = (1 << GPUIndex);
}

bool DeviceContext::IsDedicated()
{
	return GPUType == EGPUType::Dedicated_Linked || GPUType == EGPUType::Dedicated;
}

bool DeviceContext::IsIntergrated()
{
#if !BUILD_SHIPPING
	//development builds can use warp as IGPU
	return GPUType == EGPUType::Intergrated || GPUType == EGPUType::Software;
#else
	return GPUType == EGPUType::Intergrated;
#endif
}

EGPUType::Type DeviceContext::GetType()
{
	return GPUType;
}

void DeviceContext::OnFrameStart()
{}

void DeviceContext::OnFrameEnd_PreSubmit()
{}

void DeviceContext::PostInit()
{
	PerfManager::Get()->AddTimer(("TransferBytes" + std::to_string(GetDeviceIndex())).c_str(), "GPU Data");
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

int DeviceContext::GetDeviceIndex() const
{
	return DeviceIndex;
}

int DeviceContext::GetCpuFrameIndex() const
{
	return CurrentFrameIndex;
}

std::string EGPUType::ToString(EGPUType::Type type)
{
	switch (type)
	{
		case EGPUType::Dedicated:
			return "Dedicated";
		case EGPUType::Intergrated:
			return "Intergrated";
		case EGPUType::Software:
			return "Software";
	}
	return "UNKNOWN";
}

const char * ERayTracingSupportType::ToString(ERayTracingSupportType::Type e)
{
	switch (e)
	{
		case ERayTracingSupportType::Hardware:
			return "Hardware";
		case ERayTracingSupportType::DriverBased:
			return "Driver Based";
		case ERayTracingSupportType::None:
			return "None";
	}
	return "NONE";
}

const char * EMGPUConnectionMode::ToString(EMGPUConnectionMode::Type e)
{
	switch (e)
	{
		case EMGPUConnectionMode::None:
			return "Not Supported";
		case EMGPUConnectionMode::HostStagedTransfer:
			return "Host Staged GPU Transfer";
		case EMGPUConnectionMode::DirectTransfer:
			return "Direct GPU transfer (PCIE)";
		case EMGPUConnectionMode::AcceleratedDirectTransfer:
			return "Accelerated Direct GPU transfer";
	}
	return "Unknown";
}
