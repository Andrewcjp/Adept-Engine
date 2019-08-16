#include "DXMemoryManager.h"
#include "GPUResource.h"
#include "D3D12DeviceContext.h"
#include "GPUMemoryPage.h"
static ConsoleVariable LogAllocations("VMEM.LogPages", 1, ECVarType::ConsoleAndLaunch);
DXMemoryManager::DXMemoryManager(D3D12DeviceContext * D)
{
	Device = D;
	DeviceMemoryData Stats = D->GetMemoryData();
	Log::LogMessage("Booting On Device With " + StringUtils::ByteToGB(Stats.LocalSegment_TotalBytes) + "Local " +
		StringUtils::ByteToGB(Stats.HostSegment_TotalBytes) + "Host");

	AllocDesc A = AllocDesc(1024 * 1024 * 10, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	A.PageAllocationType = EPageTypes::BuffersOnly;
	A.Name = "Scratch Space Page";
	AllocPage(A, &StructScratchSpace);

	AddFrameBufferPage(1000 * 1e6);
	AddUploadPage(100 * 1e6);
	AddMeshDataPage(100 * 1e6);
}

void DXMemoryManager::AddFrameBufferPage(int size)
{
	AllocDesc A = AllocDesc(size);
	A.PageAllocationType = EPageTypes::RTAndDS_Only;
	A.Name = "Framebuffer page";
	GPUMemoryPage* Page = nullptr;
	AllocPage(A, &Page);
	FrameResourcePages.push_back(Page);
}

void DXMemoryManager::AddUploadPage(int size)
{
	AllocDesc A = AllocDesc(size);
	A.PageAllocationType = EPageTypes::BufferUploadOnly;
	A.Name = "Upload page";
	GPUMemoryPage* Page = nullptr;
	AllocPage(A, &Page);
	UploadPages.push_back(Page);
}

void DXMemoryManager::AddMeshDataPage(int size)
{
	AllocDesc A = AllocDesc(size);
	A.PageAllocationType = EPageTypes::BuffersOnly;
	A.Name = "Mesh Data page";
	GPUMemoryPage* Page = nullptr;
	AllocPage(A, &Page);
	MeshDataPages.push_back(Page);
}

DXMemoryManager::~DXMemoryManager()
{
	MemoryUtils::DeleteVector(Pages);
}

EAllocateResult::Type DXMemoryManager::AllocTemporary(AllocDesc & desc, GPUResource ** ppResource)
{
	desc.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(desc.Size, desc.Flags);
	EAllocateResult::Type Error = StructScratchSpace->Allocate(desc, ppResource);
	ensure(Error == EAllocateResult::OK);
	return Error;
}

EAllocateResult::Type DXMemoryManager::AllocFrameBuffer(AllocDesc & desc, GPUResource ** ppResource)
{
	EAllocateResult::Type Error = FrameResourcePages[0]->Allocate(desc, ppResource);
	ensure(Error == EAllocateResult::OK);
	return Error;
}

EAllocateResult::Type DXMemoryManager::AllocForUpload(AllocDesc & desc, GPUResource ** ppResource)
{
	EAllocateResult::Type Error = UploadPages[0]->Allocate(desc, ppResource);
	ensure(Error == EAllocateResult::OK);
	return Error;
}

EAllocateResult::Type DXMemoryManager::AllocMeshData(AllocDesc & desc, GPUResource ** ppResource)
{
	EAllocateResult::Type Error = MeshDataPages[0]->Allocate(desc, ppResource);
	ensure(Error == EAllocateResult::OK);
	return Error;
}

EAllocateResult::Type DXMemoryManager::AllocPage(AllocDesc & desc, GPUMemoryPage ** Page)
{
	*Page = new GPUMemoryPage(desc, Device);
	if (LogAllocations.GetBoolValue())
	{
		Log::LogMessage("Allocating Page of Size: " + StringUtils::ByteToMB(desc.Size) + " Of segment " + EGPUMemorysegment::ToString(desc.Segment));
	}
	Pages.push_back(*Page);
	//#DXMM: checks!
	return EAllocateResult::OK;
}

void DXMemoryManager::LogMemoryReport()
{
	Log::LogMessage("***GPU" + std::to_string(Device->GetDeviceIndex()) + " Memory Report***");
	for (GPUMemoryPage* P : Pages)
	{
		P->LogReport();
	}
	Log::LogMessage("***End GPU" + std::to_string(Device->GetDeviceIndex()) + " Memory Report***");
}

