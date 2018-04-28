#pragma once
#include "../RHI/RenderAPIs/D3D12/D3D12RHI.h"


class GPUSyncPoint
{
	
	HANDLE m_fenceEvent;
	ID3D12Fence* m_fence = nullptr;
	UINT64 m_fenceValue = 0;
	bool Wait = false;
public:
	GPUSyncPoint() {}
	void Init(ID3D12Device* device)
	{
		ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}
		m_fenceValue++;
	}
	void CreateSyncPoint(ID3D12CommandQueue* queue)
	{
		// Schedule a Signal command in the queue.
		ThrowIfFailed(queue->Signal(m_fence, m_fenceValue));

		// Wait until the fence has been processed.
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));
		if (WaitForSingleObject(m_fenceEvent, INFINITE) == WAIT_OBJECT_0)
		{
			// Increment the fence value for the current frame.
			m_fenceValue++;
		}
	}
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


	GPUSyncPoint GraphicsQueueSync;
	GPUSyncPoint CopyQueueSync;
	GPUSyncPoint ComputeQueueSync;
};

