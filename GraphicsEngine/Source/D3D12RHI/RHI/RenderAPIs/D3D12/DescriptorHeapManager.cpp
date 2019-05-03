#include "D3D12RHIPCH.h"
#include "DescriptorHeapManager.h"
#include "DescriptorHeap.h"
#include "Descriptor.h"


DescriptorHeapManager::DescriptorHeapManager(D3D12DeviceContext* d)
{
	Device = d;
	AllocateMainHeap(100);
}
void DescriptorHeapManager::AllocateMainHeap(int size)
{
	//setup all types
	MainHeap = new DescriptorHeap(Device, size, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	SamplerHeap = new DescriptorHeap(Device, size, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

DescriptorHeapManager::~DescriptorHeapManager()
{}

Descriptor * DescriptorHeapManager::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, int size)
{
	//handle over allocate!
	Descriptor* D = new Descriptor();
	D->Init(type, MainHeap, size);
	MainHeap->AddDescriptor(D);
	if (Device->GetDeviceIndex() == 0)
	{
		Log::LogMessage("Allocating Descriptor " + std::to_string(MainHeap->GetNumberOfDescriptors()) + "/" + std::to_string(MainHeap->GetMaxSize()));
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
