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
	void CrossGPUCreateSyncPoint_NonLocalSignal(ID3D12CommandQueue* queue, ID3D12CommandQueue* otherDeviceQeue);
	void GPUCreateSyncPoint(ID3D12CommandQueue * queue, ID3D12CommandQueue * targetqueue);
	void Signal(ID3D12CommandQueue * queue, int value = -1);
	void Wait(ID3D12CommandQueue * queue, int value = -1);
	UINT64 m_fenceValue = 0;
private:
	HANDLE m_fenceEvent = 0;
	ID3D12Fence* m_fence = nullptr;
	ID3D12Fence* secondaryFence = nullptr;

};
struct DeviceMemoryData
{
	UINT64 LocalSegment_TotalBytes = 0;
	UINT64 HostSegment_TotalBytes = 0;
	UINT64 Local_Usage = 0;
};
struct DXFeatureData
{
	D3D12_FEATURE_DATA_D3D12_OPTIONS  FeatureData;
	D3D12_FEATURE_DATA_D3D12_OPTIONS1  FeatureData1;
	D3D12_FEATURE_DATA_D3D12_OPTIONS2  FeatureData2;
	D3D12_FEATURE_DATA_D3D12_OPTIONS3  FeatureData3;
#if WIN10_1809
	D3D12_FEATURE_DATA_D3D12_OPTIONS4  FeatureData4;
	D3D12_FEATURE_DATA_D3D12_OPTIONS5  FeatureData5;
#endif
#if WIN10_1903
	D3D12_FEATURE_DATA_D3D12_OPTIONS6  FeatureData6;
#endif
	D3D12_FEATURE_DATA_ARCHITECTURE1 ArchData = {};
#if WIN10_2004
	D3D12_FEATURE_DATA_D3D12_OPTIONS7 FeatureData7 = {};
#endif
};
struct GPUUploadRequest
{
	std::vector<D3D12_SUBRESOURCE_DATA> SubResourceDesc;
	GPUResource* Target = nullptr;
	GPUResource* UploadBuffer = nullptr;
	void* DataPtr;
	size_t DataPtrSize = 0;
};
class D3D12TimeManager;
class DescriptorHeapManager;
class D3D12QueryHeap;
class DXMemoryManager;
class DescriptorCache;
class CommandAllocator;
//once this class has been completed it will be RHI split
class D3D12DeviceContext : public DeviceContext
{
public:
	D3D12DeviceContext();
	virtual ~D3D12DeviceContext();
	//Dx12 Does not differentiate  between driver based and hardware based solutions so check manually.
	bool DetectDriverDXR();

	void LogFeatureData(std::string name, bool value);

	void MoveNextFrame(int SyncIndex);
	//RHI
	void ResetDeviceAtEndOfFrame();
	void SampleVideoMemoryInfo();
	RHIClass::GPUMemoryData  GetMemoryReport();
	void DestoryDevice();
	void WaitForGpu();
	void WaitForCopy();
	void ReportData();
	//D3d12
	void CreateDeviceFromAdaptor(IDXGIAdapter1* adapter, int index);
	void LinkAdaptors(D3D12DeviceContext * other);
	ID3D12Device* GetDevice();
	ID3D12Device2 * GetDevice2();
#if WIN10_1809
	ID3D12Device5 *GetDevice5();
#endif
#if WIN10_1903
	ID3D12Device6* GetDevice6();
#endif
	ID3D12CommandQueue* GetCommandQueue();
	CopyCMDListType* GetCopyList();
	D3D12CommandList * GetCopyListDx();
	void NotifyWorkForCopyEngine();
	void UpdateCopyEngine();
	void ResetCopyEngine();
	void ExecuteComputeCommandList(ID3D12GraphicsCommandList * list);
	void ExecuteCopyCommandList(CopyCMDListType * list);
	void ExecuteInterGPUCopyCommandList(ID3D12GraphicsCommandList * list, bool forceblock = false);
	void ExecuteCommandList(ID3D12GraphicsCommandList* list);
	class RHITimeManager* GetTimeManager()override;
	void GPUWaitForOtherGPU(DeviceContext * OtherGPU, EDeviceContextQueue::Type WaitingQueue, EDeviceContextQueue::Type SignalQueue);
	void InsertCrossGPUWait(EDeviceContextQueue::Type WaitingQueue, DeviceContext* SignalingGPU, EDeviceContextQueue::Type SignalQueue);
	bool SupportsCommandList4();
	void CPUWaitForAll();
	ID3D12CommandQueue * GetCommandQueueFromEnum(EDeviceContextQueue::Type value);
	void InsertGPUWait(EDeviceContextQueue::Type WaitingQueue, EDeviceContextQueue::Type SignalQueue);
	RHICommandList* GetInterGPUCopyList();
	DescriptorHeapManager* GetHeapManager();
	DescriptorCache * GetDescriptorCache();
	D3D12QueryHeap* GetTimeStampHeap();
	D3D12QueryHeap* GetCopyTimeStampHeap();

	virtual void OnFrameStart() override;
	bool IsPartOfNodeGroup();
	int GetNodeCount();
	void CheckNVAPISupport();
	void CreateNodeDevice(ID3D12Device * dev, int nodemask, int index);
	DXMemoryManager* GetMemoryManager();
	DeviceMemoryData GetMemoryData();
	D3D_SHADER_MODEL GetShaderModel()const;
	D3D12QueryHeap* GetPipelinePerfHeap() const { return PipelinePerfHeap; }

	void EnqueueUploadRequest(const GPUUploadRequest & request);
	CommandAllocator* GetAllocator(D3D12CommandList* list);
	const DXFeatureData& GetFeatureData()const;
	bool IsUMA() const override;
	std::string ReportDeviceData() override;
private:
	DXFeatureData DeviceFeatureData;
	void FlushUploadQueue();
	D3D_SHADER_MODEL HighestShaderModel = D3D_SHADER_MODEL_5_1;
	DXMemoryManager* MemoryManager = nullptr;
	//Query heaps
	D3D12QueryHeap* TimeStampHeap = nullptr;
	D3D12QueryHeap* CopyTimeStampHeap = nullptr;

	D3D12QueryHeap* PipelinePerfHeap = nullptr;

	GPUFenceSync GraphicsSync;
	GPUFenceSync CopySync;
	GPUFenceSync InterGPUSync;
	GPUFenceSync ComputeSync;
	void CheckFeatures();
	void LogDeviceData(const std::string& data);
	void LogTierData(const std::string& data, int teir, const std::string & extramsg = "");
	void InitDevice(int index);
	bool LogDeviceDebug = true;

	//Device Data
#if SUPPORT_DXGI
	IDXGIAdapter3 * pDXGIAdapter = nullptr;
#endif
	ID3D12Device* m_Device = nullptr;
	ID3D12Device2* m_Device2 = nullptr;
#if WIN10_1809
	ID3D12Device5* m_Device5 = nullptr;
#endif
#if WIN10_1903
	ID3D12Device6* m_device6 = nullptr;
#endif
#if WIN10_2004
	ID3D12Device7* m_device7 = nullptr;
#endif
	//device info
	DeviceMemoryData MemoryData;
#if SUPPORT_DXGI
	DXGI_QUERY_VIDEO_MEMORY_INFO CurrentVideoMemoryInfo;
#endif
	size_t usedVRAM = 0;
	size_t totalVRAM = 0;
	DXGI_ADAPTER_DESC1 Adaptordesc;

	ID3D12CommandQueue* m_MainCommandQueue = nullptr;
	ID3D12CommandQueue* m_CopyCommandQueue = nullptr;
	ID3D12CommandQueue* m_ComputeCommandQueue = nullptr;
	ID3D12CommandQueue* m_SharedCopyCommandQueue = nullptr;

	//Sync controllers for each queue
	GPUSyncPoint GraphicsQueueSync;
	GPUSyncPoint CopyQueueSync;
	GPUSyncPoint ComputeQueueSync;
	GPUSyncPoint GpuWaitSyncPoint;

	GPUSyncPoint CrossAdaptorSync[RHI::CPUFrameCount];
	D3D12TimeManager* TimeManager = nullptr;
	bool SupportsCmdsList4 = false;
	GPUSyncPoint GPUWaitPoints[RHI::CPUFrameCount][EDeviceContextQueue::LIMIT];
	DescriptorHeapManager* HeapManager = nullptr;
	DescriptorCache* DescriptorCacheManager = nullptr;
	std::vector<GPUUploadRequest> Requests;
	std::vector<CommandAllocator*> Allocators;
};

class D3D12GPUSyncEvent : public RHIGPUSyncEvent
{
public:
	D3D12GPUSyncEvent(EDeviceContextQueue::Type WaitingQueue, EDeviceContextQueue::Type SignalQueue, DeviceContext * device, DeviceContext* OtherDevice);
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