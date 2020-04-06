#pragma once
#include "D3D12Types.h"

class D3D12DeviceContext;
class GPUResource;
class GPUMemoryPage;
class DXMemoryManager
{
public:
	DXMemoryManager(D3D12DeviceContext* D);
	void Compact();
	~DXMemoryManager();

	EAllocateResult::Type AllocUploadTemporary(AllocDesc & desc, GPUResource** ppResource);
	EAllocateResult::Type AllocTemporaryGPU(AllocDesc & desc, GPUResource** ppResource);
	EAllocateResult::Type AllocResource(AllocDesc & desc, GPUResource ** ppResource);
	EAllocateResult::Type AllocPage(AllocDesc & desc, GPUMemoryPage** Page);
	void UpdateTotalAlloc();
	void LogMemoryReport(bool LogResources = false);
	GPUMemoryPage* FindFreePage(AllocDesc & desc, std::vector<GPUMemoryPage*>& pages);
	UINT64 GetTotalAllocated()const ;
	UINT64 GetTotalReserved()const;
	EAllocateResult::Type CreateReservedResouce(AllocDesc& desc, GPUResource** ppResource);
private:
	void AddPage(int size);
	D3D12DeviceContext* Device = nullptr;
	//all pages on this device
	std::vector<GPUMemoryPage*> AllocatedPages;
	//all pages including special ones
	std::vector<GPUMemoryPage*> TrackedPages;
	UINT64 TotalPageAllocated = 0;
	UINT64 TotalPageUsed = 0;
};

