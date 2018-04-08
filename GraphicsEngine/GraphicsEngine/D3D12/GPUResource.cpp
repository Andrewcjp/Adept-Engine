#include "stdafx.h"
#include "GPUResource.h"
#include <algorithm>

GPUResource::GPUResource()
{}
GPUResource::GPUResource(ID3D12Resource* Target)
{
	resource = Target;
}
GPUResource::GPUResource(ID3D12Resource* Target, D3D12_RESOURCE_STATES InitalState)
{
	resource = Target;
	CurrentResourceState = InitalState;
}
GPUResource::~GPUResource()
{}

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

	ThrowIfFailed( D3D12RHI::GetDevice()->CreateHeap(&heapDesc, IID_PPV_ARGS(&pHeap)));
}

void GPUResource::Evict()
{
	ensure(currentState != eResourceState::Evicted);
	ID3D12Pageable* Pageableresource = resource;
	ThrowIfFailed(D3D12RHI::GetDevice()->Evict(1, &Pageableresource));
	currentState = eResourceState::Evicted;
}

void GPUResource::MakeResident()
{
	ensure(currentState != eResourceState::Resident);
	ID3D12Pageable* Pageableresource = resource;
	ThrowIfFailed(D3D12RHI::GetDevice()->MakeResident(1, &Pageableresource));
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
void GPUResource::SetResourceState(ID3D12GraphicsCommandList* List ,D3D12_RESOURCE_STATES newstate)
{
	if (newstate != CurrentResourceState)
	{
		List->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, CurrentResourceState, newstate));
		CurrentResourceState = newstate;
	}	
}

D3D12_RESOURCE_STATES GPUResource::GetCurrentState()
{
	return CurrentResourceState;
}
