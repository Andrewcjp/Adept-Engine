#pragma once
#include "D3D12Types.h"

class GPUResource;
class D3D12DeviceContext;
struct ResourceTileMapping 
{
	int FirstSubResource = 0;
	int NumSubResoruces = 0;
};
class GPUMemoryPage
{
public:
	GPUMemoryPage(AllocDesc & desc, D3D12DeviceContext* Context);
	~GPUMemoryPage();
	//Attempts to create a 
	EAllocateResult::Type Allocate(AllocDesc & desc, GPUResource** Resource);

	void CalculateSpaceNeeded(AllocDesc & desc);

	bool CheckSpaceForResource(AllocDesc & desc);

	void Compact();
	void Defragment(uint64 NeededSize = -1);
	//removes a resource from this page and frees its memory
	void Deallocate(GPUResource* R);

	

	UINT64 GetSize(bool LocalOnly = false) const;
	UINT64 GetSizeInUse(bool LocalOnly = false)const;
	void LogReport(bool ReportResources);
	float GetFragmentationPC();
	bool GetIsReserved() const { return IsReserved; }
	void SetReserved(bool val) { IsReserved = val; }
	//!danger! This will deallocate all of this page.
	void ResetPage();
	void EvictPage();
	void MakeResident();
	const AllocDesc & GetDesc()const;
	struct AllocationChunk
	{
		uint64 offset = 0;
		uint64 size = 0;
		bool CanFitAllocation(const AllocDesc & desc,uint64 AddressAlignmentForce)const;
	};
	bool AreChunksNext(AllocationChunk* a, AllocationChunk* b);
	void FreeChunk(AllocationChunk* R);
	void  MapResouce(GPUResource* Resource,const ResourceTileMapping & mapping);
	void MapResource2(GPUResource * Resource);
private:
	
	AllocationChunk* FindFreeChunk(AllocDesc & desc);
	AllocationChunk* AllocateFromFreeChunk(AllocDesc& desc);
	AllocationChunk* GetChunk(AllocDesc & desc);
	void CreateResource(AllocationChunk* chunk, AllocDesc & desc, ID3D12Resource ** Resource);
	void CreateResourceCommitted(AllocationChunk * chunk, AllocDesc & desc, ID3D12Resource ** Resource);
	void InitHeap();
	D3D12_HEAP_FLAGS GetFlagsForType(EPageTypes::Type T);
	ID3D12Heap* PageHeap = nullptr;
	AllocDesc PageDesc;
	std::vector<GPUResource*> ContainedResources;
	D3D12DeviceContext* Device = nullptr;
	bool IsReleaseing = false;
	std::vector<AllocationChunk*> AllocatedChunks;
	std::vector<AllocationChunk*> FreeChunks;
	bool IsReserved = false;
	bool IsResident = true;
	uint64 AddressAlignmentForce = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
};

