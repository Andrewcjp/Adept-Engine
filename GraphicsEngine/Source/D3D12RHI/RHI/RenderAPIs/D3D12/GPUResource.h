#pragma once
#include "D3D12RHI.h"
#include "D3D12Types.h"
#include "GPUMemoryPage.h"
class GPUMemoryPage;
class D3D12CommandList;
namespace EPhysicalMemoryState
{
	enum Type
	{
		Active,//we have memory and the contents are defined
		Inactive,//memory in use for another resource
		Active_NoClear,//we have memory but the contents are unknown.
	};
}
struct ResourceMipInfo
{
	UINT heapIndex;
	bool packedMip;
	bool mapped;
	D3D12_TILED_RESOURCE_COORDINATE startCoordinate;
	D3D12_TILE_REGION_SIZE regionSize;
	GPUMemoryPage::AllocationChunk* pChunk = nullptr;
};
struct ResourceTileInfo
{
	UINT heapIndex = 0;
	bool packedMip = false;
	bool mapped = false;
	bool isCurrentMapped = false;
	D3D12_TILED_RESOURCE_COORDINATE startCoordinate;
	D3D12_TILE_REGION_SIZE regionSize;
	GPUMemoryPage::AllocationChunk* pChunk = nullptr;
};
class GPUResource : public IRHIResourse
{
public:

	GPUResource();
	GPUResource(ID3D12Resource* Target, D3D12_RESOURCE_STATES InitalState);
	GPUResource(ID3D12Resource* Target, D3D12_RESOURCE_STATES InitalState, DeviceContext* Device);
	~GPUResource();
	void SetName(LPCWSTR name);

	void Evict();
	void MakeResident();
	bool IsResident();
	EResourcePageState::Type GetState();
	bool IsValidStateForList(D3D12CommandList* List);
	void SetResourceState(D3D12CommandList* List, D3D12_RESOURCE_STATES newstate, EResourceTransitionMode::Type Mode = EResourceTransitionMode::Direct, int SubResource = -1);
	//void SetResourceState(ID3D12GraphicsCommandList * List, D3D12_RESOURCE_STATES newstate, bool QueueTranstion = false);
	D3D12_RESOURCE_STATES GetCurrentState();
	ID3D12Resource* GetResource();
	void Release() override;
	void StartResourceTransition(D3D12CommandList* List, D3D12_RESOURCE_STATES newstate);
	void EndResourceTransition(D3D12CommandList* List, D3D12_RESOURCE_STATES newstate);
	bool IsTransitioning();
	void SetGPUPage(GPUMemoryPage* page);
	void UpdateUnTrackedState(D3D12_RESOURCE_STATES newstate);
	//store a ptr to the Chunk in the page for this resource.
	GPUMemoryPage::AllocationChunk* Chunk = nullptr;

	EPhysicalMemoryState::Type GetCurrentAliasState() const;
	void SetCurrentAliasState(EPhysicalMemoryState::Type val);
	bool NeedsClear() const;
	void NotifyClearComplete();
	D3D12_RESOURCE_DESC GetDesc() const { return Desc; }
	void SetDesc(D3D12_RESOURCE_DESC val) { Desc = val; }
	bool IsBacked()const { return Backed; };
	void SetBacked(bool state) { Backed = state; };
	ResourceMipInfo* GetMipData(int index);
	glm::ivec3 GetSizeAtMip();
	void FlushTileMappings();
	void SetTileMappingState(glm::ivec3 Pos, int SubResource, bool state);
	void SetTileMappingStateForSubResource(int SubResource, bool state);
	void SetTileMappingStateUV(glm::vec3 Pos, int SubResource, bool state);
	void SetupTileMappings(bool SeperateAllTiles = false);
	void SetupMipMapping();
	std::vector<ResourceTileInfo> m_Tiles;
	std::vector<ResourceTileInfo*> m_TilesToUpdate;
	int PackedMipsIndex = 0;
private:
	D3D12_TILE_SHAPE tileShape;
	std::vector<ResourceMipInfo> m_mips;
	bool Backed = true;
	D3D12_RESOURCE_DESC Desc;
	EResourcePageState::Type currentState = EResourcePageState::Resident;
	ID3D12Resource* resource = nullptr;
	D3D12_RESOURCE_STATES CurrentResourceState = {};
	D3D12_RESOURCE_STATES TargetState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
	class D3D12DeviceContext* Device;
	GPUMemoryPage* Page = nullptr;
	EPhysicalMemoryState::Type CurrentAliasState = EPhysicalMemoryState::Active;
	struct SubResouceState 
	{
		D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_COMMON;
		int Index = 0;
	};
	std::vector<SubResouceState> SubResourceStates;
	bool SubresouceSyncOut = false;
};

