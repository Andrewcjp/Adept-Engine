#pragma once
#include "D3D12RHI.h"
#include <vector>

#define _KB(x) (x * 1024)
#define _MB(x) (x * 1024 * 1024)

#define TILE_SIZE _KB(64)
#define MAX_HEAP_SIZE _MB(16)

struct GPUMemoryBlock
{
	std::vector<ID3D12Heap*> Heaps;
};


class GPUResource
{
public:
	enum eResourceState
	{
		Resident,
		Tranfering,
		DestructQueued,
		Evicted
	};
	GPUResource();
	GPUResource(ID3D12Resource * Target);
	GPUResource(ID3D12Resource * Target, D3D12_RESOURCE_STATES InitalState);
	~GPUResource();

	void CreateHeap();



	void Evict();
	void MakeResident();
	bool IsResident();
	eResourceState GetState();
	void SetResourceState(ID3D12GraphicsCommandList * List, D3D12_RESOURCE_STATES newstate);
	D3D12_RESOURCE_STATES GetCurrentState();
	ID3D12Resource* GetResource();
	void Release();
	void StartResourceTransition(ID3D12GraphicsCommandList * List, D3D12_RESOURCE_STATES newstate);
	void EndResourceTransition(ID3D12GraphicsCommandList * List, D3D12_RESOURCE_STATES newstate);
	bool IsTransitioning();
private:
	eResourceState currentState;
	ID3D12Resource* resource = nullptr;
	D3D12_RESOURCE_STATES CurrentResourceState = {};
	GPUMemoryBlock Block;
	D3D12_RESOURCE_STATES TargetState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
};

