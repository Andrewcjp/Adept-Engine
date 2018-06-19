#include "Stdafx.h"
#include "DeviceContext.h"
#include "../Core/Asserts.h"
#include "RenderAPIs/D3D12/D3D12Helpers.h"
#include "RenderAPIs/D3D12/D3D12TimeManager.h"
DeviceContext::DeviceContext()
{}

DeviceContext::~DeviceContext()
{
	WaitForGpu();
	if (m_Device)
	{
		m_Device->Release();
		for (int i = 0; i < RHI::CPUFrameCount; i++)
		{
			m_commandAllocator[i]->Release();
			m_commandAllocator[i] = nullptr;
		}
		m_commandQueue->Release();
		m_commandQueue = nullptr;
	}
	if (m_CopyCommandAllocator)
	{
		m_CopyList->Release();
		m_CopyCommandAllocator->Release();
		m_CopyCommandQueue->Release();
	}
	pDXGIAdapter->Release();
	delete TimeManager;
	/*if (pDXGIAdapter != nullptr)
	{
		pDXGIAdapter->UnregisterVideoMemoryBudgetChangeNotification(m_BudgetNotificationCookie);
	}*/
}

void DeviceContext::CreateDeviceFromAdaptor(IDXGIAdapter1 * adapter, int index)
{

	pDXGIAdapter = (IDXGIAdapter3*)adapter;
	HRESULT result = D3D12CreateDevice(
		pDXGIAdapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_Device)
	);
	ensureMsgf(!(result == DXGI_ERROR_UNSUPPORTED), "D3D_FEATURE_LEVEL_11_0 is required to run this engine");
	ThrowIfFailed(result);

	D3D_FEATURE_LEVEL MaxLevel = D3D12RHI::GetMaxSupportedFeatureLevel(m_Device);
	if (MaxLevel != D3D_FEATURE_LEVEL_11_0)
	{
		m_Device->Release();
		ThrowIfFailed(D3D12CreateDevice(
			pDXGIAdapter,
			MaxLevel,
			IID_PPV_ARGS(&m_Device)
		));
	}
	if (LogDeviceDebug)
	{
		std::cout << "Device Created With Feature level " << D3D12Helpers::StringFromFeatureLevel(MaxLevel) << std::endl;
	}
	DeviceIndex = index;
	D3D12RHI::CheckFeatures(m_Device);
#if 0
	pDXGIAdapter->RegisterVideoMemoryBudgetChangeNotificationEvent(m_VideoMemoryBudgetChange, &m_BudgetNotificationCookie);
#endif

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
	m_commandQueue->SetName(L"Core Device Command Queue");
	for (int i = 0; i < RHI::CPUFrameCount; i++)
	{
		ThrowIfFailed(m_Device->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(&m_commandAllocator[i])));
		m_commandAllocator[i]->SetName(L"Core Device Allocator");
	}

	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CopyCommandQueue)));
	ThrowIfFailed(m_Device->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(&m_CopyCommandAllocator)));
	
	ThrowIfFailed(m_Device->CreateCommandList(0, queueDesc.Type, m_CopyCommandAllocator, nullptr, IID_PPV_ARGS(&m_CopyList)));
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		
	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_SharedCopyCommandQueue)));
	for (int i = 0; i < RHI::CPUFrameCount; i++)
	{
		ThrowIfFailed(m_Device->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(&m_SharedCopyCommandAllocator[i])));
	}
	ThrowIfFailed(m_Device->CreateCommandList(0, queueDesc.Type, m_SharedCopyCommandAllocator[0], nullptr, IID_PPV_ARGS(&m_IntraCopyList)));
	m_SharedCopyCommandQueue->SetName(L"m_SharedCopyCommandQueue");
	m_IntraCopyList->Close();

	GraphicsQueueSync.Init(GetDevice());
	CopyQueueSync.Init(GetDevice());
	GpuWaitSyncPoint.InitGPUOnly(GetDevice());
	D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
	ThrowIfFailed(GetDevice()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, reinterpret_cast<void*>(&options), sizeof(options)));
	//todo: validate the device capablities 
	//GetDevice()->SetStablePowerState(false);

	TimeManager = new D3D12TimeManager(this);
}
void DeviceContext::LinkAdaptors(DeviceContext* other)
{
	CrossAdaptorSync.Init(GetDevice(), other->GetDevice());
}
ID3D12Device * DeviceContext::GetDevice()
{
	return m_Device;
}

ID3D12CommandAllocator * DeviceContext::GetCommandAllocator()
{
	return m_commandAllocator[CurrentFrameIndex];
}

ID3D12CommandAllocator * DeviceContext::GetSharedCommandAllocator()
{
	return m_SharedCopyCommandAllocator[CurrentFrameIndex];
}

ID3D12CommandQueue * DeviceContext::GetCommandQueue()
{
	return m_commandQueue;
}

void DeviceContext::SampleVideoMemoryInfo()
{
	pDXGIAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &CurrentVideoMemoryInfo);

	usedVRAM = CurrentVideoMemoryInfo.CurrentUsage / 1024 / 1024;
	totalVRAM = CurrentVideoMemoryInfo.Budget / 1024 / 1024;
}

std::string DeviceContext::GetMemoryReport()
{
	std::string output = "VMEM: ";
	output.append(std::to_string(usedVRAM));
	output.append("MB / ");
	output.append(std::to_string(totalVRAM));
	output.append("MB");
	return output;
}
void DeviceContext::MoveNextFrame()
{
	CurrentFrameIndex++;
	if (CurrentFrameIndex == RHI::CPUFrameCount - 1)
	{
		CurrentFrameIndex = 0;
	}
}
void DeviceContext::DestoryDevice()
{

}

void DeviceContext::WaitForGpu()
{
	GraphicsQueueSync.CreateSyncPoint(m_commandQueue);
}
void DeviceContext::WaitForCopy()
{
	CopyQueueSync.CreateSyncPoint(m_SharedCopyCommandQueue);
}
ID3D12GraphicsCommandList * DeviceContext::GetCopyList()
{
	return m_CopyList;
}

ID3D12GraphicsCommandList * DeviceContext::GetSharedCopyList()
{
	return m_IntraCopyList;
}

void DeviceContext::ResetSharingCopyList()
{	
	ThrowIfFailed(m_IntraCopyList->Reset(GetSharedCommandAllocator(), nullptr));
}

void DeviceContext::NotifyWorkForCopyEngine()
{
	CopyEngineHasWork = true;
}

void DeviceContext::UpdateCopyEngine()
{
	if (CopyEngineHasWork)
	{
		ThrowIfFailed(m_CopyList->Close());
		ExecuteCopyCommandList(m_CopyList);
		/*ThrowIfFailed(m_CopyCommandAllocator->Reset());
		ThrowIfFailed(m_CopyList->Reset(m_CopyCommandAllocator, nullptr));*/
		CopyEngineHasWork = false;
	}
	
}


void DeviceContext::ExecuteCopyCommandList(ID3D12GraphicsCommandList * list)
{
	ID3D12CommandList* ppCommandLists[] = { list };
	m_CopyCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	CopyQueueSync.CreateSyncPoint(m_CopyCommandQueue);
}

void DeviceContext::ExecuteInterGPUCopyCommandList(ID3D12GraphicsCommandList * list,bool forceblock)
{
	ID3D12CommandList* ppCommandLists[] = { list };
	m_SharedCopyCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (RHI::BlockCommandlistExec() || forceblock)
	{
		GraphicsQueueSync.CreateSyncPoint(m_SharedCopyCommandQueue);
	}

}
void DeviceContext::ExecuteCommandList(ID3D12GraphicsCommandList * list)
{

	ID3D12CommandList* ppCommandLists[] = { list };
	GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (RHI::BlockCommandlistExec())
	{
		WaitForGpu();
	}
}

void DeviceContext::StartExecuteCommandList(ID3D12GraphicsCommandList * list)
{
	ID3D12CommandList* ppCommandLists[] = { list };
	GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	GraphicsQueueSync.CreateStartSyncPoint(m_commandQueue);
}

void DeviceContext::EndExecuteCommandList()
{
	GraphicsQueueSync.WaitOnSync();
}

int DeviceContext::GetDeviceIndex()
{
	return DeviceIndex;
}

D3D12TimeManager * DeviceContext::GetTimeManager()
{
	return TimeManager;
}

int DeviceContext::GetCpuFrameIndex()
{
	return CurrentFrameIndex;
}

void DeviceContext::GPUWaitForOtherGPU(DeviceContext * OtherGPU)
{
	CrossAdaptorSync.CrossGPUCreateSyncPoint(GetCommandQueue(), OtherGPU->GetCommandQueue());
}

void DeviceContext::InsertGPUWait()
{
	GpuWaitSyncPoint.GPUCreateSyncPoint(GetCommandQueue(), m_SharedCopyCommandQueue);
}
void DeviceContext::InsertGPUWaitForSharedCopy()
{
	GpuWaitSyncPoint.GPUCreateSyncPoint(m_SharedCopyCommandQueue, GetCommandQueue());
}
GPUSyncPoint::~GPUSyncPoint()
{
	if (m_fence)
	{
		m_fence->Release();
		m_fence = nullptr;
	}
}

void GPUSyncPoint::Init(ID3D12Device * device, ID3D12Device* SecondDevice)
{
	//Fence types
	//  D3D12_FENCE_FLAG_NONE
	//  D3D12_FENCE_FLAG_SHARED
	//	D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER
	//	D3D12_FENCE_FLAG_NON_MONITORED
	ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_SHARED | D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER, IID_PPV_ARGS(&m_fence)));
	m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (m_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
	m_fenceValue++;
	// Create a shared handle to the cross adapter fence
	HANDLE fenceHandle = nullptr;
	device->CreateSharedHandle(
		m_fence,
		nullptr,
		GENERIC_ALL,
		nullptr,
		&fenceHandle);

	// Open shared handle to fence on secondaryDevice GPU
	SecondDevice->OpenSharedHandle(fenceHandle, IID_PPV_ARGS(&secondaryFence));

	CloseHandle(fenceHandle);
}
void GPUSyncPoint::InitGPUOnly(ID3D12Device * device)
{
	ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValue++;
}
void GPUSyncPoint::Init(ID3D12Device * device)
{
	//Fence types
	//  D3D12_FENCE_FLAG_NONE
	//  D3D12_FENCE_FLAG_SHARED
	//	D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER
	//	D3D12_FENCE_FLAG_NON_MONITORED
	ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (m_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
	m_fenceValue++;
}

void GPUSyncPoint::CreateSyncPoint(ID3D12CommandQueue * queue)
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
void GPUSyncPoint::CrossGPUCreateSyncPoint(ID3D12CommandQueue * queue, ID3D12CommandQueue* otherDeviceQeue)
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(queue->Signal(m_fence, m_fenceValue));

	otherDeviceQeue->Wait(secondaryFence, m_fenceValue);
	m_fenceValue++;
}
void GPUSyncPoint::GPUCreateSyncPoint(ID3D12CommandQueue * queue, ID3D12CommandQueue * targetqueue)
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(queue->Signal(m_fence, m_fenceValue));

	targetqueue->Wait(m_fence, m_fenceValue);
	m_fenceValue++;
}
void GPUSyncPoint::CreateStartSyncPoint(ID3D12CommandQueue * queue)
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(queue->Signal(m_fence, m_fenceValue));
	ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));
	DidStartWork = true;
}

void GPUSyncPoint::WaitOnSync()
{
	if (!DidStartWork)
	{
		std::cout << "ERROR:Wait Called on Non Async List\n";
		return;
	}
	// Wait until the fence has been processed.	
	if (WaitForSingleObject(m_fenceEvent, INFINITE) == WAIT_OBJECT_0)
	{
		// Increment the fence value for the current frame.
		m_fenceValue++;
		DidStartWork = false;
	}
}

