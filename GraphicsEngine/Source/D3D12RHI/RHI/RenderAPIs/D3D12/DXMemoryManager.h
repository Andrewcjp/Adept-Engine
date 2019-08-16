#pragma once
#include "D3D12Types.h"

class D3D12DeviceContext;
class GPUResource;
class GPUMemoryPage;
class DXMemoryManager
{
public:
	DXMemoryManager(D3D12DeviceContext* D);
	void AddFrameBufferPage(int size);
	void AddUploadPage(int size);
	void AddMeshDataPage(int size);
	~DXMemoryManager();
	EAllocateResult::Type AllocTemporary(AllocDesc & desc, GPUResource** ppResource);
	EAllocateResult::Type AllocFrameBuffer(AllocDesc & desc, GPUResource ** ppResource);
	EAllocateResult::Type AllocForUpload(AllocDesc & desc, GPUResource ** ppResource);
	EAllocateResult::Type AllocMeshData(AllocDesc & desc, GPUResource ** ppResource);
	EAllocateResult::Type AllocPage(AllocDesc & desc, GPUMemoryPage** Page);
	void LogMemoryReport();
private:
	D3D12DeviceContext* Device = nullptr;
	//all pages on this device
	std::vector<GPUMemoryPage*> Pages;
	GPUMemoryPage* StructScratchSpace = nullptr;
	//pages used for frame buffers Constant buffers etc.
	std::vector<GPUMemoryPage*> FrameResourcePages;
	//texture pages only
	std::vector<GPUMemoryPage*> TexturePages;

	std::vector<GPUMemoryPage*> UploadPages;
	std::vector<GPUMemoryPage*> MeshDataPages;
};

