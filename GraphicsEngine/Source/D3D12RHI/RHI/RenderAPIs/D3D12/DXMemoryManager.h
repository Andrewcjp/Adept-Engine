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
	void AddFrameBufferPage(int size);
	void AddTransientPage(int size);
	void AddTransientGPUOnlyPage(int size);
	void AddTexturePage(int size);
	void AddDataPage(int size);
	~DXMemoryManager();

	EAllocateResult::Type AllocTemporary(AllocDesc & desc, GPUResource** ppResource);
	EAllocateResult::Type AllocTemporaryGPU(AllocDesc & desc, GPUResource** ppResource);
	EAllocateResult::Type AllocGeneral(AllocDesc & desc, GPUResource** ppResource);
	EAllocateResult::Type AllocFrameBuffer(AllocDesc & desc, GPUResource ** ppResource);
	EAllocateResult::Type AllocTexture(AllocDesc & desc, GPUResource ** ppResource);
	EAllocateResult::Type AllocPage(AllocDesc & desc, GPUMemoryPage** Page);
	void UpdateTotalAlloc();
	void LogMemoryReport();
	GPUMemoryPage* FindFreePage(AllocDesc & desc, std::vector<GPUMemoryPage*>& pages);
private:
	D3D12DeviceContext* Device = nullptr;
	//all pages on this device
	std::vector<GPUMemoryPage*> Pages;
	//pages used for frame buffers Constant buffers etc.
	std::vector<GPUMemoryPage*> FrameResourcePages;
	//texture pages only
	std::vector<GPUMemoryPage*> TexturePages;
	std::vector<GPUMemoryPage*> TempUploadPages;
	//All other data
	std::vector<GPUMemoryPage*> DataPages;
	//pages that used for GPU only scratch spaces etc.
	std::vector<GPUMemoryPage*> TempGPUPages;

	UINT64 TotalPageAllocated = 0;
	UINT64 TotalPageUsed = 0;
};

