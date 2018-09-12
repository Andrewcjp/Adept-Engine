#include "stdafx.h"
#include "GPUResource.h"
#include <algorithm>
#include "D3D12DeviceContext.h"
CreateChecker(GPUResource);
GPUResource::GPUResource()
{}

GPUResource::GPUResource(ID3D12Resource* Target, D3D12_RESOURCE_STATES InitalState) :GPUResource(Target, InitalState, (D3D12DeviceContext*)RHI::GetDefaultDevice())
{}

GPUResource::GPUResource(ID3D12Resource * Target, D3D12_RESOURCE_STATES InitalState, DeviceContext * device)
{
	AddCheckerRef(GPUResource, this);
	resource = Target;
	NAME_D3D12_OBJECT(Target);
	CurrentResourceState = InitalState;
	Device = (D3D12DeviceContext*)device;
}

GPUResource::~GPUResource()
{
	if (!IsReleased)
	{
		Release();
	}
}

void GPUResource::SetName(LPCWSTR name)
{
	resource->SetName(name);
}

void GPUResource::CreateHeap()
{

	Block.Heaps.push_back(nullptr);

	ID3D12Heap*  pHeap = Block.Heaps[0];
	int RemainingSize = 1 * TILE_SIZE;
	D3D12_HEAP_DESC heapDesc = {};

	heapDesc.SizeInBytes = std::min(RemainingSize, MAX_HEAP_SIZE);
	heapDesc.Alignment = 0;
	heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//
	// Tier 1 heaps have restrictions on the type of information that can be stored in
	// a heap. To accommodate this, we will retsrict the content to only shader resources.
	// The heap cannot store textures that are used as render targets, depth-stencil
	// output, or buffers. But this is okay, since we do not use these heaps for those
	// purposes.
	//
	heapDesc.Flags = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_BUFFERS;

	//ThrowIfFailed( D3D12RHI::GetDevice()->CreateHeap(&heapDesc, IID_PPV_ARGS(&pHeap)));
}

void GPUResource::Evict()
{
	ensure(currentState != eResourceState::Evicted);
	ID3D12Pageable* Pageableresource = resource;
	ThrowIfFailed(Device->GetDevice()->Evict(1, &Pageableresource));
	currentState = eResourceState::Evicted;
}

void GPUResource::MakeResident()
{
	ensure(currentState != eResourceState::Resident);
	ID3D12Pageable* Pageableresource = resource;
	ThrowIfFailed(Device->GetDevice()->MakeResident(1, &Pageableresource));
	currentState = eResourceState::Resident;
}

bool GPUResource::IsResident()
{
	return (currentState == eResourceState::Resident);
}

GPUResource::eResourceState GPUResource::GetState()
{
	return currentState;
}

void GPUResource::SetResourceState(ID3D12GraphicsCommandList* List, D3D12_RESOURCE_STATES newstate)
{
	if (newstate != CurrentResourceState)
	{
		List->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, CurrentResourceState, newstate));
		CurrentResourceState = newstate;
		TargetState = newstate;
	}
}
//todo More Detailed Error checking!
void GPUResource::StartResourceTransition(ID3D12GraphicsCommandList * List, D3D12_RESOURCE_STATES newstate)
{
	if (newstate != CurrentResourceState)
	{
		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
		BarrierDesc.Transition.StateBefore = CurrentResourceState;
		BarrierDesc.Transition.StateAfter = newstate;
		BarrierDesc.Transition.pResource = resource;
		List->ResourceBarrier(1, &BarrierDesc);
		TargetState = newstate;
	}
}

void GPUResource::EndResourceTransition(ID3D12GraphicsCommandList * List, D3D12_RESOURCE_STATES newstate)
{
	if (newstate != CurrentResourceState)
	{
		D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
		BarrierDesc.Transition.StateBefore = CurrentResourceState;
		BarrierDesc.Transition.StateAfter = newstate;
		BarrierDesc.Transition.pResource = resource;
		List->ResourceBarrier(1, &BarrierDesc);
		CurrentResourceState = newstate;
	}
}
bool GPUResource::IsTransitioning()
{
	return (CurrentResourceState != TargetState);
}
D3D12_RESOURCE_STATES GPUResource::GetCurrentState()
{
	return CurrentResourceState;
}

ID3D12Resource * GPUResource::GetResource()
{
	return resource;
}

void GPUResource::Release()
{
	IRHIResourse::Release();
	SafeRelease(resource);
	RemoveCheckerRef(GPUResource, this);
}


