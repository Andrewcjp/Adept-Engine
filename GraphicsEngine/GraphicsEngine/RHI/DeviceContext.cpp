#include "Stdafx.h"
#include "DeviceContext.h"


DeviceContext::DeviceContext()
{}


DeviceContext::~DeviceContext()
{}

void DeviceContext::CreateDeviceFromAdaptor(IDXGIAdapter1 * adapter)
{
	pDXGIAdapter = (IDXGIAdapter3*)adapter;
	ThrowIfFailed(D3D12CreateDevice(
		pDXGIAdapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_Device)
	));
	

	//GetMaxSupportedFeatureLevel(m_Primarydevice);

#if 0
	pDXGIAdapter->RegisterVideoMemoryBudgetChangeNotificationEvent(m_VideoMemoryBudgetChange, &m_BudgetNotificationCookie);
#endif
	
	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
	ThrowIfFailed(m_Device->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(&m_commandAllocator)));


	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CopyCommandQueue)));
	ThrowIfFailed(m_Device->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(&m_CopyCommandAllocator)));

	GraphicsQueueSync.Init(GetDevice());
	CopyQueueSync.Init(GetDevice());

}

ID3D12Device * DeviceContext::GetDevice()
{
	return m_Device;
}

ID3D12CommandAllocator * DeviceContext::GetCommandAllocator()
{
	return m_commandAllocator;
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

void DeviceContext::DestoryDevice()
{
	WaitForGpu();//Ensure the GPU has complete the current task!
	m_Device->Release();
	m_Device = nullptr;
	m_commandAllocator->Release();
	m_commandAllocator = nullptr;
	m_commandQueue->Release();
	m_commandQueue = nullptr;	
}

void DeviceContext::WaitForGpu()
{
	GraphicsQueueSync.CreateSyncPoint(m_commandQueue);
}

GPUSyncPoint::~GPUSyncPoint()
{
	if (m_fence)
	{
		m_fence->Release();
		m_fence = nullptr;
	}
}

void GPUSyncPoint::Init(ID3D12Device * device)
{
	//Fence types
	//  D3D12_FENCE_FLAG_NONE
	//  D3D12_FENCE_FLAG_SHARED
	//	D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER
	//	D3D12_FENCE_FLAG_NON_MONITORED
	ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceEvent = CreateEvent(nullptr, false,false, nullptr);
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
