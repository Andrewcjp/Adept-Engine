#include "DXMemoryManager.h"
#include "GPUResource.h"
#include "D3D12DeviceContext.h"
#include "GPUMemoryPage.h"
#include "Core\Maths\Math.h"
static ConsoleVariable LogAllocations("VMEM.LogPages", 1, ECVarType::ConsoleAndLaunch);
DXMemoryManager::DXMemoryManager(D3D12DeviceContext * D)
{
	Device = D;
	DeviceMemoryData Stats = D->GetMemoryData();
	Log::LogMessage("Booting On Device With " + StringUtils::ByteToGB(Stats.LocalSegment_TotalBytes) + "Local " +
		StringUtils::ByteToGB(Stats.HostSegment_TotalBytes) + "Host");

	AddFrameBufferPage(Math::MBToBytes<int>(10));
	AddTransientPage(Math::MBToBytes<int>(10));
	AddDataPage(Math::MBToBytes<int>(10));
	AddTexturePage(Math::MBToBytes<int>(10));
	AddTransientGPUOnlyPage(Math::MBToBytes<int>(10));
}

void DXMemoryManager::Compact()
{
	for (int i = Pages.size() - 1; i >= 0; i--)
	{
		if (Pages[i]->GetSizeInUse() == 0)
		{
			VectorUtils::Remove(FrameResourcePages, Pages[i]);
			VectorUtils::Remove(TempUploadPages, Pages[i]);
			VectorUtils::Remove(TempGPUPages, Pages[i]);
			VectorUtils::Remove(DataPages, Pages[i]);
			VectorUtils::Remove(TexturePages, Pages[i]);
			SafeDelete(Pages[i]);
			Pages.erase(Pages.begin() + i);
		}
	}
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

void DXMemoryManager::AddTransientPage(int size)
{
	AllocDesc A = AllocDesc(size);
	A.PageAllocationType = EPageTypes::BufferUploadOnly;
	A.Name = "Upload page";
	GPUMemoryPage* Page = nullptr;
	AllocPage(A, &Page);
	TempUploadPages.push_back(Page);
}

void DXMemoryManager::AddTransientGPUOnlyPage(int size)
{
	AllocDesc A = AllocDesc(size);
	A.PageAllocationType = EPageTypes::BuffersOnly;
	A.Name = "GPU Temp data page";
	GPUMemoryPage* Page = nullptr;
	AllocPage(A, &Page);
	TempGPUPages.push_back(Page);
}

void DXMemoryManager::AddTexturePage(int size)
{
	AllocDesc A = AllocDesc(size);
	A.PageAllocationType = EPageTypes::TexturesOnly;
	A.Name = "Texture page";
	GPUMemoryPage* Page = nullptr;
	AllocPage(A, &Page);
	TexturePages.push_back(Page);
}

void DXMemoryManager::AddDataPage(int size)
{
	AllocDesc A = AllocDesc(size);
	A.PageAllocationType = EPageTypes::BuffersOnly;
	A.Name = "General data page";
	GPUMemoryPage* Page = nullptr;
	AllocPage(A, &Page);
	DataPages.push_back(Page);
}

DXMemoryManager::~DXMemoryManager()
{
	MemoryUtils::DeleteVector(Pages);
}

EAllocateResult::Type DXMemoryManager::AllocUploadTemporary(AllocDesc & desc, GPUResource ** ppResource)
{
	desc.PageAllocationType = EPageTypes::BufferUploadOnly;
	EAllocateResult::Type Error = FindFreePage(desc, TempUploadPages)->Allocate(desc, ppResource);
	ensure(Error == EAllocateResult::OK);
	return Error;
}

EAllocateResult::Type DXMemoryManager::AllocTemporaryGPU(AllocDesc & desc, GPUResource ** ppResource)
{
	desc.PageAllocationType = EPageTypes::BuffersOnly;
	EAllocateResult::Type Error = FindFreePage(desc, TempGPUPages)->Allocate(desc, ppResource);
	ensure(Error == EAllocateResult::OK);
	return Error;
}

EAllocateResult::Type DXMemoryManager::AllocGeneral(AllocDesc & desc, GPUResource ** ppResource)
{
	desc.PageAllocationType = EPageTypes::BuffersOnly;
	EAllocateResult::Type Error = FindFreePage(desc, DataPages)->Allocate(desc, ppResource);
	ensure(Error == EAllocateResult::OK);
	return Error;
}

EAllocateResult::Type DXMemoryManager::AllocFrameBuffer(AllocDesc & desc, GPUResource ** ppResource)
{
	desc.PageAllocationType = EPageTypes::RTAndDS_Only;
	EAllocateResult::Type Error = FindFreePage(desc, FrameResourcePages)->Allocate(desc, ppResource);
	ensure(Error == EAllocateResult::OK);
	return Error;
}

EAllocateResult::Type DXMemoryManager::AllocTexture(AllocDesc & desc, GPUResource ** ppResource)
{
	desc.PageAllocationType = EPageTypes::TexturesOnly;
	EAllocateResult::Type Error = FindFreePage(desc, TexturePages)->Allocate(desc, ppResource);
	ensure(Error == EAllocateResult::OK);
	return Error;
}

EAllocateResult::Type DXMemoryManager::AllocPage(AllocDesc & desc, GPUMemoryPage ** Page)
{
	*Page = new GPUMemoryPage(desc, Device);
	if (LogAllocations.GetBoolValue())
	{
		//Log::LogMessage("Allocating Page of Size: " + StringUtils::ByteToMB(desc.Size) + " Of segment " + EGPUMemorysegment::ToString(desc.Segment));
	}
	Pages.push_back(*Page);
	//#DXMM: checks!
	return EAllocateResult::OK;
}

void DXMemoryManager::UpdateTotalAlloc()
{
	TotalPageAllocated = 0;
	TotalPageUsed = 0;
	for (GPUMemoryPage* P : Pages)
	{
		TotalPageAllocated += P->GetSize();
		TotalPageUsed += P->GetSizeInUse();
	}
}


void DXMemoryManager::LogMemoryReport()
{
	UpdateTotalAlloc();
	Log::LogMessage("***GPU" + std::to_string(Device->GetDeviceIndex()) + " Memory Report***");
	for (GPUMemoryPage* P : Pages)
	{
		P->LogReport();
	}
	Log::LogMessage("Total " + StringUtils::ByteToMB(TotalPageUsed) + " of " + StringUtils::ByteToMB(TotalPageAllocated) + " allocated (" +
		StringUtils::ToString(((float)TotalPageUsed / TotalPageAllocated) * 100) + "%)");

	Log::LogMessage("Device Total " + StringUtils::ByteToMB(TotalPageAllocated) + " of " + StringUtils::ByteToMB(Device->GetMemoryData().LocalSegment_TotalBytes) +
		" (" + StringUtils::ToString(((float)TotalPageAllocated / Device->GetMemoryData().LocalSegment_TotalBytes) * 100) + "%)");
	Log::LogMessage("***End GPU" + std::to_string(Device->GetDeviceIndex()) + " Memory Report***");
}

GPUMemoryPage * DXMemoryManager::FindFreePage(AllocDesc & desc, std::vector<GPUMemoryPage*>& pages)
{
	for (int i = 0; i < pages.size(); i++)
	{
		pages[i]->CalculateSpaceNeeded(desc);
		if (pages[i]->CheckSpaceForResource(desc))
		{
			return pages[i];
		}
	}
	GPUMemoryPage* newpage = nullptr;
	desc.Size = desc.TextureAllocData.SizeInBytes;
	AllocPage(desc, &newpage);
	pages.push_back(newpage);
	return newpage;
}

UINT64 DXMemoryManager::GetTotalAllocated() const
{
	return TotalPageAllocated;
}

