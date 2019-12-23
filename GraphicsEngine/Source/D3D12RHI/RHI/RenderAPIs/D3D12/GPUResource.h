#pragma once
#include "D3D12RHI.h"
#include "D3D12Types.h"
#include "GPUMemoryPage.h"
class GPUMemoryPage;
class D3D12CommandList;

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
	void SetResourceState(D3D12CommandList * List, D3D12_RESOURCE_STATES newstate, bool QueueTranstion = false);
	void SetResourceState(ID3D12GraphicsCommandList * List, D3D12_RESOURCE_STATES newstate, bool QueueTranstion = false);
	D3D12_RESOURCE_STATES GetCurrentState();
	ID3D12Resource* GetResource();
	void Release() override;
	void StartResourceTransition(D3D12CommandList * List, D3D12_RESOURCE_STATES newstate);
	void EndResourceTransition(D3D12CommandList * List, D3D12_RESOURCE_STATES newstate);
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

