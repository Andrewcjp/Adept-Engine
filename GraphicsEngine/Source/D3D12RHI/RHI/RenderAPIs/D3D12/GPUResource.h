#pragma once
#include "D3D12RHI.h"
#include "D3D12Types.h"
#include "GPUMemoryPage.h"

class GPUMemoryPage;
class D3D12CommandList;


#define _KB(x) (x * 1024)
#define _MB(x) (x * 1024 * 1024)

#define TILE_SIZE _KB(64)
#define MAX_HEAP_SIZE _MB(16)

class GPUResource : public IRHIResourse
{
public:

	GPUResource();
	GPUResource(ID3D12Resource * Target, D3D12_RESOURCE_STATES InitalState);
	GPUResource(ID3D12Resource * Target, D3D12_RESOURCE_STATES InitalState, DeviceContext* Device);
	~GPUResource();
	void SetName(LPCWSTR name);

	void Evict();
	void MakeResident();
	bool IsResident();
	EResourcePageState::Type GetState();
	bool IsValidStateForList(D3D12CommandList * List);
	void SetResourceState(ID3D12GraphicsCommandList * List, D3D12_RESOURCE_STATES newstate);
	D3D12_RESOURCE_STATES GetCurrentState();
	ID3D12Resource* GetResource();
	void Release() override;
	void StartResourceTransition(ID3D12GraphicsCommandList * List, D3D12_RESOURCE_STATES newstate);
	void EndResourceTransition(ID3D12GraphicsCommandList * List, D3D12_RESOURCE_STATES newstate);
	bool IsTransitioning();
	void SetGPUPage(GPUMemoryPage* page);
	void UpdateUnTrackedState(D3D12_RESOURCE_STATES newstate);
	//store a ptr to the Chunk in the page for this resource.
	GPUMemoryPage::AllocationChunk* Chunk = nullptr;
private:
	EResourcePageState::Type currentState = EResourcePageState::Resident;
	ID3D12Resource* resource = nullptr;
	D3D12_RESOURCE_STATES CurrentResourceState = {};
	D3D12_RESOURCE_STATES TargetState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
	class D3D12DeviceContext* Device;
	GPUMemoryPage* Page = nullptr;
};

