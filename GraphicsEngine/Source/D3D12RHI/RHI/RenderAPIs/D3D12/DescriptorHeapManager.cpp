#include "DescriptorHeapManager.h"
#include "DescriptorHeap.h"
#include "DXDescriptor.h"
#include "D3D12RHI.h"
#include "RHI\RHICommandList.h"
#include "RHI\DeviceContext.h"
#include "D3D12CommandList.h"
#include "D3D12DeviceContext.h"
#include "Core\Performance\PerfManager.h"
#include "DescriptorCache.h"

DescriptorHeapManager::DescriptorHeapManager(D3D12DeviceContext* d)
{
	Device = d;
	AllocateMainHeap(2000);//resize broken
	PerfManager::Get()->AddTimer(TimerName, "RHI");
}
void DescriptorHeapManager::AllocateMainHeap(int size)
{
	//setup all types
	MainHeap = new DescriptorHeap(Device, size, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	SamplerHeap = new DescriptorHeap(Device, 512, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

DescriptorHeapManager::~DescriptorHeapManager()
{
	SafeRelease(MainHeap);
	SafeRelease(SamplerHeap);
}

DXDescriptor * DescriptorHeapManager::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, int size)
{
	//const int HeapIndex = index;
	//if (MainHeap[HeapIndex]->GetNextFreeIndex() + size >= MainHeap[HeapIndex]->GetMaxSize())
	//{
	//	Reallocate(&MainHeap[HeapIndex], MainHeap[HeapIndex]->GetMaxSize() + std::max(250, size));
	//	DidJustResize = true;
	//}
	////handle over allocate!
	//DXDescriptor* D = new DXDescriptor();
	//D->Init(type, MainHeap[HeapIndex], size);
	//MainHeap[HeapIndex]->AddDescriptor(D);
	//return D;
	return nullptr;
}

void DescriptorHeapManager::CheckAndRealloc(int size, D3D12CommandList* list)
{
	int index = 0;
	if (MainHeap->GetNextFreeIndex() + size >= MainHeap->GetMaxSize())
	{
		const int newsize = MainHeap->GetMaxSize() + std::max(100, size);
		Log::LogMessage("Out of Space in main heap Reallocating was " + std::to_string(MainHeap->GetMaxSize()) + " expanded to " + std::to_string(newsize));
		Reallocate(&MainHeap, newsize);
		//we need to invalidate the cache as current written descs could be in the old heap
		Device->GetDescriptorCache()->Invalidate();
	}
	list->AddHeap(MainHeap);
}
void DescriptorHeapManager::ClearMainHeap()
{
	PerfManager::AddToCountTimer(TimerName, GetMainHeap()->GetNumberOfDescriptorsForStats());
	if (Device->GetCpuFrameIndex() == 0)
	{
		GetMainHeap()->ClearHeap();
	}
	else
	{
		GetMainHeap()->SetFrameGuardBound();
	}
}

DescriptorHeap * DescriptorHeapManager::GetMainHeap()
{
	return MainHeap;
}

void DescriptorHeapManager::BindHeap(D3D12CommandList * list)
{
	MainHeap->BindHeap(list);
}

void DescriptorHeapManager::RebindHeap(D3D12CommandList* list)
{
	if (DidJustResize)
	{
		BindHeap(list);
		DidJustResize = false;
	}
}
void DescriptorHeapManager::CheckAndRealloc(DescriptorHeap** Target, int num)
{
	DescriptorHeap* Current = *Target;
	if (Current->GetNextFreeIndex() + num >= Current->GetMaxSize())
	{
		Reallocate(Target, Current->GetMaxSize() + 50);
	}
}

void DescriptorHeapManager::Reallocate(DescriptorHeap** TargetHeap, int newsize)
{
	Log::LogMessage("Reallocating heap was " + std::to_string(TargetHeap[0]->GetMaxSize()) + " expanded to " + std::to_string(newsize));
	DescriptorHeap* OldHeap = *TargetHeap;
	DescriptorHeap* newheap = new DescriptorHeap(OldHeap, newsize);
	//OldHeap->MoveAllToHeap(newheap);
	RHI::AddToDeferredDeleteQueue(OldHeap);
	*TargetHeap = newheap;

}

void DescriptorHeapManager::EndOfFrame()
{}
