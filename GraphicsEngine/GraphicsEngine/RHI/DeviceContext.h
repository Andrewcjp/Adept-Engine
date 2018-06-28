#pragma once
#include "RHI/RenderAPIs/D3D12/D3D12RHI.h"

#include <mutex>
#include <queue>
template<class T>
class ThreadSafe_Queue
{
public:
	std::queue<T> q;
	std::mutex m;

	void push(T item)
	{
		std::lock_guard<std::mutex> lock(m);
		q.push(item);
	}
	T Pop()
	{
		if (q.empty())
		{
			return nullptr;
		}
		std::lock_guard<std::mutex> lock(m);
		T elem = q.front();
		q.pop();
		return elem;
	}
	bool IsEmpty()
	{
		return q.empty();
	}

};


class GPUSyncPoint
{
public:
	GPUSyncPoint() {}
	~GPUSyncPoint();
	void Init(ID3D12Device * device, ID3D12Device * SecondDevice);
	void InitGPUOnly(ID3D12Device* device);
	void Init(ID3D12Device* device);
	void CreateSyncPoint(ID3D12CommandQueue* queue);
	void CrossGPUCreateSyncPoint(ID3D12CommandQueue * queue, ID3D12CommandQueue * otherDeviceQeue);
	void GPUCreateSyncPoint(ID3D12CommandQueue * queue, ID3D12CommandQueue * targetqueue);
	void CreateStartSyncPoint(ID3D12CommandQueue* queue);
	void WaitOnSync();
	
private:
	HANDLE m_fenceEvent;
	ID3D12Fence* m_fence = nullptr;
	ID3D12Fence* secondaryFence = nullptr;
	
	UINT64 m_fenceValue = 0;
	bool DidStartWork = false;
};

//once this class has been completed it will be RHI split
class DeviceContext
{
public:
	DeviceContext();
	~DeviceContext();
	void CreateDeviceFromAdaptor(IDXGIAdapter1* adapter, int index);
	void LinkAdaptors(DeviceContext * other);
	ID3D12Device* GetDevice();
	ID3D12CommandAllocator* GetCommandAllocator();
	ID3D12CommandAllocator * GetSharedCommandAllocator();
	ID3D12CommandQueue* GetCommandQueue();

	void SampleVideoMemoryInfo();
	std::string GetMemoryReport();
	void MoveNextFrame();
	void DestoryDevice();
	void WaitForGpu();
	void WaitForCopy();
	ID3D12GraphicsCommandList* GetCopyList();
	ID3D12GraphicsCommandList * GetSharedCopyList();
	void ResetSharingCopyList();
	void NotifyWorkForCopyEngine();
	void UpdateCopyEngine();
	void ExecuteCopyCommandList(ID3D12GraphicsCommandList * list);
	void ExecuteInterGPUCopyCommandList(ID3D12GraphicsCommandList * list, bool forceblock = false);
	void ExecuteCommandList(ID3D12GraphicsCommandList* list);
	void StartExecuteCommandList(ID3D12GraphicsCommandList* list);
	void EndExecuteCommandList();
	int GetDeviceIndex();
	class D3D12TimeManager* GetTimeManager();
	int GetCpuFrameIndex();
	void GPUWaitForOtherGPU(DeviceContext * OtherGPU, DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue);
	int CurrentFrameIndex = 0;
	void CPUWaitForAll();
	ID3D12CommandQueue * GetCommandQueueFromEnum(DeviceContextQueue::Type value);
	void InsertGPUWait(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue);
	void WaitForGPU(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue);

	RHICommandList* GetInterGPUCopyList();

private:	
	bool LogDeviceDebug = true;
	int DeviceIndex = 0;
	
	//Device Data
	IDXGIAdapter3 * pDXGIAdapter = nullptr;
	ID3D12Device* m_Device = nullptr;
	ID3D12CommandAllocator* m_commandAllocator[RHI::CPUFrameCount];
	ID3D12CommandQueue* m_commandQueue = nullptr;
	

	

	//device info
	DXGI_QUERY_VIDEO_MEMORY_INFO CurrentVideoMemoryInfo;
	size_t usedVRAM = 0;
	size_t totalVRAM = 0;
	
	//Copy List for this GPU
	ID3D12GraphicsCommandList* m_CopyList = nullptr;
	ID3D12CommandAllocator* m_CopyCommandAllocator = nullptr;
	ID3D12CommandQueue* m_CopyCommandQueue = nullptr;
	ID3D12GraphicsCommandList* m_IntraCopyList= nullptr;
	ID3D12CommandAllocator* m_SharedCopyCommandAllocator[RHI::CPUFrameCount] = { nullptr };
	ID3D12CommandQueue* m_SharedCopyCommandQueue = nullptr;
	//Sync controllers for each queue
	GPUSyncPoint GraphicsQueueSync;
	GPUSyncPoint CopyQueueSync;
	GPUSyncPoint ComputeQueueSync;
	GPUSyncPoint GpuWaitSyncPoint;
	GPUSyncPoint CrossAdaptorSync;

	D3D12TimeManager* TimeManager = nullptr;
	//copy queue management 
	bool CopyEngineHasWork = false;


	RHICommandList* GPUCopyList = nullptr;
};

