#pragma once
#include "D3D12Types.h"

class D3D12DeviceContext;
class GPUResource;
class GPUMemoryPage;
class DXMemoryManager
{
public:
	DXMemoryManager(D3D12DeviceContext* D);
	~DXMemoryManager();
	EAllocateResult::Type AllocTemporary(AllocDesc & desc, GPUResource** ppResource);
	EAllocateResult::Type AllocPage(AllocDesc & desc, GPUMemoryPage** Page);
private:
	D3D12DeviceContext* Device = nullptr;
	//all pages on this device
	std::vector<GPUMemoryPage*> Pages;
	GPUMemoryPage* StructScratchSpace = nullptr;
	//pages used for frame buffers Constant buffers etc.
	std::vector<GPUMemoryPage*> FrameResourcePages;
	//texture pages only
	std::vector<GPUMemoryPage*> TexturePages;
};

