#pragma once
#include "../RHI/RenderAPIs/D3D12/D3D12RHI.h"

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

	void Init(ID3D12Device* device);
	void CreateSyncPoint(ID3D12CommandQueue* queue);
	void CreateStartSyncPoint(ID3D12CommandQueue* queue);
	void WaitOnSync();
private:
	HANDLE m_fenceEvent;
	ID3D12Fence* m_fence = nullptr;
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
	ID3D12Device* GetDevice();
	ID3D12CommandAllocator* GetCommandAllocator();
	ID3D12CommandQueue* GetCommandQueue();

	void SampleVideoMemoryInfo();
	std::string GetMemoryReport();
	void DestoryDevice();
	void EnsureWorkComplete();
	void WaitForGpu();
	ID3D12GraphicsCommandList* GetCopyList();
	void NotifyWorkForCopyEngine();
	void UpdateCopyEngine();
	void ExecuteCopyCommandList(ID3D12GraphicsCommandList * list);
	void ExecuteCommandList(ID3D12GraphicsCommandList* list);
	void StartExecuteCommandList(ID3D12GraphicsCommandList* list);
	void EndExecuteCommandList();
	//void Wait(ID3D12GraphicsCommandList* list);
	int GetDeviceIndex();
	static DWORD WINAPI StartThread(void* param)
	{
		DeviceContext* This = (DeviceContext*)param;
		return This->WorkerThreadMain();
	}
	//todo: might be a slight issue here
	//relating to present before everything is finished
	DWORD WorkerThreadMain()
	{		
		while (WorkerRunning)
		{
			
			if (!CommandLists.IsEmpty())
			{
				ResetEvent(WorkerThreadEnd);
				ID3D12CommandList* ppCommandLists[] = { CommandLists.Pop() };
				GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
				WaitForGpu();
			}
			else
			{
				SetEvent(WorkerThreadEnd);
			}
		}
		return 0;
	}

private:	
	bool LogDeviceDebug = true;
	int DeviceIndex = 0;
	//Device Data
	IDXGIAdapter3 * pDXGIAdapter = nullptr;
	ID3D12Device* m_Device = nullptr;
	ID3D12CommandAllocator* m_commandAllocator = nullptr;
	ID3D12CommandQueue* m_commandQueue = nullptr;
	
	//device info
	DXGI_QUERY_VIDEO_MEMORY_INFO CurrentVideoMemoryInfo;
	size_t usedVRAM = 0;
	size_t totalVRAM = 0;
	
	//async commanndlists
	ThreadSafe_Queue<ID3D12GraphicsCommandList*> CommandLists;

	HANDLE WorkerThreadEnd;
	bool WorkerRunning = true;
	//Copy List for this GPU
	ID3D12GraphicsCommandList* m_CopyList = nullptr;
	ID3D12CommandAllocator* m_CopyCommandAllocator = nullptr;
	ID3D12CommandQueue* m_CopyCommandQueue = nullptr;

	//Sync controllers for each queue
	GPUSyncPoint GraphicsQueueSync;
	GPUSyncPoint CopyQueueSync;
	GPUSyncPoint ComputeQueueSync;

	//copy queue management 
	bool CopyEngineHasWork = false;
};

