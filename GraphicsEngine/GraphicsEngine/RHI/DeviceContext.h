#pragma once
#include "../RHI/RenderAPIs/D3D12/D3D12RHI.h"

class GPUSyncPoint
{
public:
	GPUSyncPoint() {}
	~GPUSyncPoint();

	void Init(ID3D12Device* device);
	void CreateSyncPoint(ID3D12CommandQueue* queue);

private:
	HANDLE m_fenceEvent;
	ID3D12Fence* m_fence = nullptr;
	UINT64 m_fenceValue = 0;
};
//once this class has been completed it will be RHI split
class DeviceContext
{
public:
	DeviceContext();
	~DeviceContext();

	void CreateDeviceFromAdaptor(IDXGIAdapter1* adapter);
	ID3D12Device* GetDevice();
	ID3D12CommandAllocator* GetCommandAllocator();
	ID3D12CommandQueue* GetCommandQueue();

	void SampleVideoMemoryInfo();
	std::string GetMemoryReport();
	void DestoryDevice();
	void WaitForGpu();

private:	
	//Device Data
	IDXGIAdapter3 * pDXGIAdapter = nullptr;
	ID3D12Device* m_Device = nullptr;
	ID3D12CommandAllocator* m_commandAllocator = nullptr;
	ID3D12CommandQueue* m_commandQueue = nullptr;
	
	//device info
	DXGI_QUERY_VIDEO_MEMORY_INFO CurrentVideoMemoryInfo;
	size_t usedVRAM = 0;
	size_t totalVRAM = 0;
	
	//Copy List for this GPU
	ID3D12GraphicsCommandList* m_CopyList = nullptr;
	ID3D12CommandAllocator* m_CopyCommandAllocator = nullptr;
	ID3D12CommandQueue* m_CopyCommandQueue = nullptr;

	//Sync controllers for each queue
	GPUSyncPoint GraphicsQueueSync;
	GPUSyncPoint CopyQueueSync;
	GPUSyncPoint ComputeQueueSync;
};

