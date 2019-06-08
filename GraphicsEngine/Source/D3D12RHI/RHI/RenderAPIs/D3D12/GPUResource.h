#pragma once
#include "D3D12RHI.h"
#include "D3D12Types.h"

class GPUMemoryPage;

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
	EResourceState::Type GetState();
	void SetResourceState(ID3D12GraphicsCommandList * List, D3D12_RESOURCE_STATES newstate);
	D3D12_RESOURCE_STATES GetCurrentState();
	ID3D12Resource* GetResource();
	void Release() override;
	void StartResourceTransition(ID3D12GraphicsCommandList * List, D3D12_RESOURCE_STATES newstate);
	void EndResourceTransition(ID3D12GraphicsCommandList * List, D3D12_RESOURCE_STATES newstate);
	bool IsTransitioning();
private:
	EResourceState::Type currentState = EResourceState::Resident;
	ID3D12Resource* resource = nullptr;
	D3D12_RESOURCE_STATES CurrentResourceState = {};
	D3D12_RESOURCE_STATES TargetState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
	class D3D12DeviceContext* Device;
	GPUMemoryPage* Page = nullptr;
};

