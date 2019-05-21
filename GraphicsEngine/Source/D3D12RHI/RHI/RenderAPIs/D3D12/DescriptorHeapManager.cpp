#include "D3D12RHIPCH.h"
#include "DescriptorHeapManager.h"
#include "DescriptorHeap.h"
#include "Descriptor.h"
#include "D3D12RHI.h"


DescriptorHeapManager::DescriptorHeapManager(D3D12DeviceContext* d)
{
	Device = d;
	AllocateMainHeap(250);
}
void DescriptorHeapManager::AllocateMainHeap(int size)
{
	//setup all types
	MainHeap = new DescriptorHeap(Device, size, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	SamplerHeap = new DescriptorHeap(Device, size, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

DescriptorHeapManager::~DescriptorHeapManager()
{
	SafeRelease(MainHeap);
	SafeRelease(SamplerHeap);
}

Descriptor * DescriptorHeapManager::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, int size)
{
	if (MainHeap->GetNextFreeIndex() + size >= MainHeap->GetMaxSize())
	{
		Reallocate(&MainHeap, MainHeap->GetMaxSize() + std::max(10, size));
	}
	//handle over allocate!
	Descriptor* D = new Descriptor();
	D->Init(type, MainHeap, size);
	MainHeap->AddDescriptor(D);
	if (Device->GetDeviceIndex() == 0)
	{
		Log::LogMessage("Allocating Descriptor " + std::to_string(MainHeap->GetNextFreeIndex()) + "/" + std::to_string(MainHeap->GetMaxSize()));
	}
	return D;
}

DescriptorHeap * DescriptorHeapManager::GetMainHeap()
{
	return MainHeap;
}

void DescriptorHeapManager::BindHeap(D3D12CommandList * list)
{
	MainHeap->BindHeap(list);
}

void DescriptorHeapManager::Reallocate(DescriptorHeap** TargetHeap, int newsize)
{
	DescriptorHeap* OldHeap = *TargetHeap;
	DescriptorHeap* newheap = new DescriptorHeap(OldHeap, newsize);
	OldHeap->MoveAllToHeap(newheap);
	RHI::AddToDeferredDeleteQueue(OldHeap);
	*TargetHeap = newheap;
}
