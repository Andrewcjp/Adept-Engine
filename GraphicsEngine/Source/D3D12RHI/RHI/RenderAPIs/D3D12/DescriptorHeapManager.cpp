#include "D3D12RHIPCH.h"
#include "DescriptorHeapManager.h"
#include "DescriptorHeap.h"
#include "Descriptor.h"
#include "D3D12RHI.h"
#include "DescriptorGroup.h"
#include "RHI\RHICommandList.h"
#include "RHI\DeviceContext.h"
#include "D3D12CommandList.h"
#include "D3D12DeviceContext.h"

DescriptorHeapManager::DescriptorHeapManager(D3D12DeviceContext* d)
{
	Device = d;
	AllocateMainHeap(300);
}
void DescriptorHeapManager::AllocateMainHeap(int size)
{
	//setup all types
	MainHeap[0] = new DescriptorHeap(Device, size, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	MainHeap[1] = new DescriptorHeap(Device, size, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	SamplerHeap = new DescriptorHeap(Device, size, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

DescriptorHeapManager::~DescriptorHeapManager()
{
	SafeRelease(MainHeap[0]);
	SafeRelease(MainHeap[1]);
	SafeRelease(SamplerHeap);
}

Descriptor * DescriptorHeapManager::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, int size)
{
	//	ensure(false);
	if (MainHeap[0]->GetNextFreeIndex() + size >= MainHeap[0]->GetMaxSize())
	{
		Reallocate(&MainHeap[0], MainHeap[0]->GetMaxSize() + std::max(10, size));
	}
	//handle over allocate!
	Descriptor* D = new Descriptor();
	D->Init(type, MainHeap[0], size);
	MainHeap[0]->AddDescriptor(D);
	if (Device->GetDeviceIndex() == 0)
	{
		Log::LogMessage("Allocating Descriptor " + std::to_string(MainHeap[0]->GetNextFreeIndex()) + "/" + std::to_string(MainHeap[0]->GetMaxSize()));
	}
	return D;
}

DescriptorGroup * DescriptorHeapManager::AllocateDescriptorGroup(D3D12_DESCRIPTOR_HEAP_TYPE type, int size)
{
	CheckAndRealloc(size);
	DescriptorGroup* D = new DescriptorGroup();
	D->Init(type, MainHeap[0], size);
	MainHeap[0]->AddDescriptor(D->GetDescriptor(0));
	MainHeap[1]->AddDescriptor(D->GetDescriptor(1));
	Groups.push_back(D);
	return D;
}

void DescriptorHeapManager::CheckAndRealloc(int size)
{
	if (MainHeap[0]->GetNextFreeIndex() + size >= MainHeap[0]->GetMaxSize())
	{
		const int newsize = MainHeap[0]->GetMaxSize() + std::max(10, size);
		Log::LogMessage("Out of Space in main heap Reallocating was " + std::to_string(MainHeap[0]->GetMaxSize()) + " expanded to " + std::to_string(newsize));
		Reallocate(&MainHeap[Device->GetCpuFrameIndex()], newsize);
		if (RHI::GetFrameCount() == 0)
		{
			Reallocate(&MainHeap[1], newsize);
		}
	}
}

DescriptorHeap * DescriptorHeapManager::GetMainHeap()
{
	return MainHeap[0];
}

void DescriptorHeapManager::BindHeap(D3D12CommandList * list)
{
	MainHeap[list->GetDevice()->GetCpuFrameIndex()]->BindHeap(list);
}

void DescriptorHeapManager::Reallocate(DescriptorHeap** TargetHeap, int newsize)
{
	DescriptorHeap* OldHeap = *TargetHeap;
	DescriptorHeap* newheap = new DescriptorHeap(OldHeap, newsize);
	OldHeap->MoveAllToHeap(newheap);
	RHI::AddToDeferredDeleteQueue(OldHeap);
	*TargetHeap = newheap;
}

void DescriptorHeapManager::EndOfFrame()
{
	for (int i = 0; i < Groups.size(); i++)
	{
		Groups[i]->OnFrameSwitch();
	}
}
