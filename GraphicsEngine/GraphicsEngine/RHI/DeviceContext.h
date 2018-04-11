#pragma once
#include "../D3D12/D3D12RHI.h"
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
	void StartWaitForGpuHandle();
	void EndWaitForGpuHandle();

private:	
	IDXGIAdapter3 * pDXGIAdapter = nullptr;
	ID3D12Device* m_Device;
	ID3D12CommandAllocator* m_commandAllocator;
	ID3D12CommandQueue* m_commandQueue;

	//device info
	DXGI_QUERY_VIDEO_MEMORY_INFO CurrentVideoMemoryInfo;
	size_t usedVRAM = 0;
	size_t totalVRAM = 0;
	
	//Syncronisation
	HANDLE m_fenceEvent;
	ID3D12Fence* m_fence = nullptr;
	UINT64 m_fenceValue = 0;
	bool Wait = false;
};

