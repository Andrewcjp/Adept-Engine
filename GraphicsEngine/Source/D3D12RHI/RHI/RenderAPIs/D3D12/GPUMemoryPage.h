#pragma once
#include "D3D12Types.h"

class GPUResource;
class D3D12DeviceContext;
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
	//removes a resource from this page and frees its memory
	void Deallocate(GPUResource* R);
	UINT GetSize()const;
	void LogReport();
private:
	void CreateResource(AllocDesc & desc, ID3D12Resource ** Resource);
	void InitHeap();
	D3D12_HEAP_FLAGS GetFlagsForType(EPageTypes::Type T);
	ID3D12Heap* PageHeap = nullptr;
	AllocDesc PageDesc;
	std::vector<GPUResource*> ContainedResources;
	D3D12DeviceContext* Device = nullptr;
	UINT64 OffsetInPlacedHeap =0;
	bool IsReleaseing = false;
};

