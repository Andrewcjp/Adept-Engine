#pragma once
#include "RHI/DeviceContext.h"
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

class GPUFenceSync
{
public:
	void Init(ID3D12CommandQueue* TargetQueue, ID3D12Device* device);
	void MoveNextFrame(int SyncIndex);
	int GetFrameIndex()
	{
		return m_frameIndex;
	}
	~GPUFenceSync()
	{
		SafeRelease(m_fence);
		CloseHandle(m_fenceEvent);
	}
private:
	ID3D12CommandQueue* Queue = nullptr;
	UINT64 m_fenceValues[RHI::CPUFrameCount] = { 0 };
	ID3D12Fence* m_fence = nullptr;
	int m_frameIndex = 0;
	HANDLE m_fenceEvent;
};

class GPUSyncPoint
{
public:
	GPUSyncPoint()
	{}
	~GPUSyncPoint();
	void Init(ID3D12Device * device, ID3D12Device * SecondDevice);
	void InitGPUOnly(ID3D12Device* device);
	void Init(ID3D12Device* device);
	void CreateSyncPoint(ID3D12CommandQueue* queue);
	void CrossGPUCreateSyncPoint(ID3D12CommandQueue * queue, ID3D12CommandQueue * otherDeviceQeue);
	void GPUCreateSyncPoint(ID3D12CommandQueue * queue, ID3D12CommandQueue * targetqueue);
	void Signal(ID3D12CommandQueue * queue, int value = -1);
	void Wait(ID3D12CommandQueue * queue, int value = -1);
	UINT64 m_fenceValue = 0;
private:
	HANDLE m_fenceEvent = 0;
	ID3D12Fence* m_fence = nullptr;
	ID3D12Fence* secondaryFence = nullptr;

};
class D3D12TimeManager;
class DescriptorHeapManager;
class D3D12QueryHeap;
//once this class has been completed it will be RHI split
class D3D12DeviceContext : public DeviceContext
{
public:
	D3D12DeviceContext();
	virtual ~D3D12DeviceContext();

	void LogFeatureData(std::string name, bool value);

	void MoveNextFrame(int SyncIndex);
	//RHI
	void ResetDeviceAtEndOfFrame();
	void SampleVideoMemoryInfo();
	std::string GetMemoryReport();
	void DestoryDevice();
	void WaitForGpu();
	void WaitForCopy();
	void ReportData();
	//D3d12
	void CreateDeviceFromAdaptor(IDXGIAdapter1* adapter, int index);
	void LinkAdaptors(D3D12DeviceContext * other);
	ID3D12Device* GetDevice();
	ID3D12Device2 * GetDevice2();
	ID3D12Device5 *GetDevice5();
	ID3D12CommandAllocator* GetCommandAllocator();
	ID3D12CommandAllocator * GetComputeCommandAllocator();
	ID3D12CommandAllocator * GetCopyCommandAllocator();
	ID3D12CommandAllocator * GetSharedCommandAllocator();
	ID3D12CommandQueue* GetCommandQueue();
	ID3D12GraphicsCommandList* GetCopyList();
	ID3D12GraphicsCommandList * GetSharedCopyList();
	void ResetSharingCopyList();
	void NotifyWorkForCopyEngine();
	void UpdateCopyEngine();
	void ResetCopyEngine();
	void ExecuteComputeCommandList(ID3D12GraphicsCommandList * list);
	void ExecuteCopyCommandList(ID3D12GraphicsCommandList * list);
	void ExecuteInterGPUCopyCommandList(ID3D12GraphicsCommandList * list, bool forceblock = false);
	void ExecuteCommandList(ID3D12GraphicsCommandList* list);
	class RHITimeManager* GetTimeManager()override;
	int GetCpuFrameIndex();
	void GPUWaitForOtherGPU(DeviceContext * OtherGPU, DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue);
	bool SupportsCommandList4();
	void CPUWaitForAll();
	ID3D12CommandQueue * GetCommandQueueFromEnum(DeviceContextQueue::Type value);
	void InsertGPUWait(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue);
	void ResetWork();
	RHICommandList* GetInterGPUCopyList();
	DescriptorHeapManager* GetHeapManager();
	D3D12QueryHeap* GetTimeStampHeap();
	D3D12QueryHeap* GetCopyTimeStampHeap();

	virtual void OnFrameStart() override;
	bool IsPartOfNodeGroup();
	int GetNodeCount();
	void CreateNodeDevice(ID3D12Device * dev, int nodemask, int index);
private:
	//Query heaps
	D3D12QueryHeap* TimeStampHeap = nullptr;
	D3D12QueryHeap* CopyTimeStampHeap = nullptr;

	GPUFenceSync GraphicsSync;
	GPUFenceSync CopySync;
	GPUFenceSync InterGPUSync;
	GPUFenceSync ComputeSync;
	void CheckFeatures();
	void LogDeviceData(const std::string& data);
	void LogTierData(const std::string& data, int teir);
	void InitDevice(int index);
	bool LogDeviceDebug = true;

	//Device Data
	IDXGIAdapter3 * pDXGIAdapter = nullptr;
	ID3D12Device* m_Device = nullptr;
	ID3D12Device2* m_Device2 = nullptr;
	ID3D12Device5* m_Device5 = nullptr;
	ID3D12CommandAllocator* m_commandAllocator[RHI::CPUFrameCount] = { nullptr,nullptr };
	ID3D12CommandQueue* m_MainCommandQueue = nullptr;

	int CurrentFrameIndex = 0;

	//device info
	DXGI_QUERY_VIDEO_MEMORY_INFO CurrentVideoMemoryInfo;
	size_t usedVRAM = 0;
	size_t totalVRAM = 0;
	DXGI_ADAPTER_DESC1 Adaptordesc;

	ID3D12CommandAllocator* m_CopyCommandAllocator = nullptr;
	ID3D12CommandQueue* m_CopyCommandQueue = nullptr;
	ID3D12CommandQueue* m_ComputeCommandQueue = nullptr;
	ID3D12GraphicsCommandList* m_IntraCopyList = nullptr;
	ID3D12CommandAllocator* m_SharedCopyCommandAllocator[RHI::CPUFrameCount] = { nullptr };
	ID3D12CommandQueue* m_SharedCopyCommandQueue = nullptr;
	//Sync controllers for each queue
	GPUSyncPoint GraphicsQueueSync;
	GPUSyncPoint CopyQueueSync;
	GPUSyncPoint ComputeQueueSync;
	GPUSyncPoint GpuWaitSyncPoint;

	GPUSyncPoint CrossAdaptorSync[RHI::CPUFrameCount];
	D3D12TimeManager* TimeManager = nullptr;
	bool SupportsCmdsList4 = false;
	GPUSyncPoint GPUWaitPoints[RHI::CPUFrameCount][DeviceContextQueue::LIMIT];
	DescriptorHeapManager* HeapManager = nullptr;
};

class D3D12GPUSyncEvent : public RHIGPUSyncEvent
{
public:
	D3D12GPUSyncEvent(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue, DeviceContext * device, DeviceContext* OtherDevice);
	~D3D12GPUSyncEvent();
	void SignalWait();
	virtual void Signal() override;
	virtual void Wait() override;
private:
	GPUSyncPoint Point[RHI::CPUFrameCount];
	ID3D12CommandQueue* WaitingQueue = nullptr;
	ID3D12CommandQueue* SignalQueue = nullptr;
	DeviceContext* SignalingDevice = nullptr;
};