#include "DXMemoryManager.h"
#include "GPUResource.h"
#include "D3D12DeviceContext.h"
#include "GPUMemoryPage.h"

DXMemoryManager::DXMemoryManager(D3D12DeviceContext * D)
{
	Device = D;
	DeviceMemoryData Stats = D->GetMemoryData();
	Log::LogMessage("Booting On Device With " + StringUtils::ByteToGB(Stats.LocalSegment_TotalBytes) + "Local " +
		StringUtils::ByteToGB(Stats.HostSegment_TotalBytes) + "Host");

	AllocPage(AllocDesc(1024 * 1024 * 10, D3D12_RESOURCE_STATE_UNORDERED_ACCESS), &StructScratchSpace);
}

DXMemoryManager::~DXMemoryManager()
{
	MemoryUtils::DeleteVector(Pages);
}

EAllocateResult::Type DXMemoryManager::AllocTemporary(AllocDesc & desc, GPUResource ** ppResource)
{
	EAllocateResult::Type Error = StructScratchSpace->Allocate(desc, ppResource);
	ensure(Error == EAllocateResult::OK);
	return Error;
}

EAllocateResult::Type DXMemoryManager::AllocPage(AllocDesc & desc, GPUMemoryPage ** Page)
{
	*Page = new GPUMemoryPage(desc, Device);
	Log::LogMessage("Allocating Page of Size: " + StringUtils::ByteToMB(desc.Size) + " Of segment " + EGPUMemorysegment::ToString(desc.Segment));
	//#DXMM: checks!
	return EAllocateResult::OK;
}

