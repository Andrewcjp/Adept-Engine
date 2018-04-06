#include "stdafx.h"
#include "D3D12RHI.h"
#include "../BaseApplication.h"
#include <D3Dcompiler.h>
#include "glm\glm.hpp"
#include "include\glm\gtx\transform.hpp"
D3D12RHI* D3D12RHI::Instance = nullptr;
#include "../Rendering/Shaders/ShaderMipMap.h"
D3D12RHI::D3D12RHI()
{
	Instance = this;
}


D3D12RHI::~D3D12RHI()
{}
//todo: 
void D3D12RHI::InitContext()
{}

void D3D12RHI::DestroyContext()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForGpu();	
	pDXGIAdapter->UnregisterVideoMemoryBudgetChangeNotification(m_BudgetNotificationCookie);
	ReleaseSwapRTs();
	m_commandAllocator->Release();
	m_commandQueue->Release();
	m_Primarydevice->Release();
	CloseHandle(m_fenceEvent);
}

void EnableShaderBasedValidation()
{
	ID3D12Debug* spDebugController0;
	ID3D12Debug1* spDebugController1;
	(D3D12GetDebugInterface(IID_PPV_ARGS(&spDebugController0)));
	(spDebugController0->QueryInterface(IID_PPV_ARGS(&spDebugController1)));
	spDebugController1->SetEnableGPUBasedValidation(true);
}
void CheckFeatures(ID3D12Device* pDevice)
{
	D3D12_FEATURE_DATA_D3D12_OPTIONS FeatureData;
	ZeroMemory(&FeatureData, sizeof(FeatureData));
	HRESULT hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &FeatureData, sizeof(FeatureData));
	if (SUCCEEDED(hr))
	{
		// TypedUAVLoadAdditionalFormats contains a Boolean that tells you whether the feature is supported or not
	/*	if (FeatureData.TypedUAVLoadAdditionalFormats)
		{*/
		// Can assume “all-or-nothing” subset is supported (e.g. R32G32B32A32_FLOAT)
		// Cannot assume other formats are supported, so we check:
		D3D12_FEATURE_DATA_FORMAT_SUPPORT FormatSupport = { DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };
		hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &FormatSupport, sizeof(FormatSupport));
		if (SUCCEEDED(hr) && (FormatSupport.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE) != 0)
		{
			// DXGI_FORMAT_R32G32_FLOAT supports UAV Typed Load!
			//__debugbreak();
		}
		//}
	}
}
D3D_FEATURE_LEVEL D3D12RHI::GetMaxSupportedFeatureLevel(ID3D12Device* pDevice)
{
	D3D12_FEATURE_DATA_FEATURE_LEVELS FeatureData;
	ZeroMemory(&FeatureData, sizeof(FeatureData));
	D3D_FEATURE_LEVEL FeatureLevelsList[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_12_1,
	};
	FeatureData.pFeatureLevelsRequested = FeatureLevelsList;
	FeatureData.NumFeatureLevels = 1;
	HRESULT hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &FeatureData, sizeof(FeatureData));
	if (SUCCEEDED(hr))
	{
		return FeatureData.MaxSupportedFeatureLevel;
	}
	return D3D_FEATURE_LEVEL_11_0;
}
void D3D12RHI::DisplayDeviceDebug()
{
	IDXGIFactory4* pFactory;
	CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)&pFactory);

	IDXGIAdapter3* adapter;
	pFactory->EnumAdapters(0, reinterpret_cast<IDXGIAdapter**>(&adapter));

	DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
	adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);

	size_t usedVRAM = videoMemoryInfo.CurrentUsage / 1024 / 1024;
	size_t totalVRAM = videoMemoryInfo.Budget / 1024 / 1024;
	std::cout << "Primary Adaptor Has " << usedVRAM << "MB / " << totalVRAM << "MB" << std::endl;

}
void D3D12RHI::LoadPipeLine()
{
	//EnableShaderBasedValidation();
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG) //nsight needs this off
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif
	IDXGIFactory4* factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	if (false/*m_useWarpDevice*/)
	{
		IDXGIAdapter* warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter,
			D3D_FEATURE_LEVEL_11_1,
			IID_PPV_ARGS(&m_Primarydevice)
		));
	}


	IDXGIAdapter1* hardwareAdapter;
	GetHardwareAdapter(factory, &hardwareAdapter);

	ThrowIfFailed(D3D12CreateDevice(
		hardwareAdapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_Primarydevice)
	));

	GetMaxSupportedFeatureLevel(m_Primarydevice);

	pDXGIAdapter = (IDXGIAdapter3*)hardwareAdapter;
	pDXGIAdapter->RegisterVideoMemoryBudgetChangeNotificationEvent(m_VideoMemoryBudgetChange, &m_BudgetNotificationCookie);

	if (false)
	{
		IDXGIAdapter1* shardwareAdapter;
		ThrowIfFailed(factory->EnumAdapters1(1, &shardwareAdapter));
		//GetHardwareAdapter(factory, &shardwareAdapter);

		ThrowIfFailed(D3D12CreateDevice(
			shardwareAdapter,
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_Secondarydevice)
		));

	}
	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_Primarydevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
	DisplayDeviceDebug();
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	CheckFeatures(m_Primarydevice);
	IDXGISwapChain1* swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue,		// Swap chain needs the queue so that it can force a flush on it.
		BaseApplication::GetHWND(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));
	m_swapChain = (IDXGISwapChain3*)swapChain;

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(BaseApplication::GetHWND(), DXGI_MWA_NO_ALT_ENTER));

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_Primarydevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

		m_rtvDescriptorSize = m_Primarydevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_Primarydevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
		m_rtvDescriptorSize = m_Primarydevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	}

	CreateSwapChainRTs();
	ThrowIfFailed(m_Primarydevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}
void D3D12RHI::CreateSwapChainRTs()
{
	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_Primarydevice->CreateRenderTargetView(m_renderTargets[n], nullptr, rtvHandle);

			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
		NAME_D3D12_OBJECT(m_renderTargets[1]);
		NAME_D3D12_OBJECT(m_renderTargets[0]);
	}
}

void D3D12RHI::InitMipmaps()
{
#if USEGPUTOGENMIPS
	MipmapShader = new ShaderMipMap();
#endif
}

void D3D12RHI::InternalResizeSwapChain(int x, int y)
{
	if (m_swapChain != nullptr && HasSetup)
	{
		ReleaseSwapRTs();
		ThrowIfFailed(m_swapChain->ResizeBuffers(FrameCount, x, y, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
		CreateSwapChainRTs();
		m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(x), static_cast<float>(y));
		m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(x), static_cast<LONG>(y));
		CreateDepthStencil(x, y);
		RequestedResize = false;
	}
}
void D3D12RHI::ReleaseSwapRTs()
{
	for (UINT n = 0; n < FrameCount; n++)
	{
		m_renderTargets[n]->Release();
	}
	m_depthStencil->Release();
}
void D3D12RHI::ResizeSwapChain(int x, int y)
{
	if (m_swapChain != nullptr && HasSetup)
	{
		RequestedResize = true;
		newwidth = x;
		newheight = y;
	}
}

void D3D12RHI::CreateDepthStencil(int width, int height)
{
	//create the depth stencil for the screen
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	ThrowIfFailed(m_Primarydevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&m_depthStencil)
	));

	NAME_D3D12_OBJECT(m_depthStencil);

	m_Primarydevice->CreateDepthStencilView(m_depthStencil, &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}
void D3D12RHI::LoadAssets()
{

	{
		ThrowIfFailed(m_Primarydevice->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValues[m_frameIndex]++;
		//	m_fenceValue = 1;


		// Create an event handle to use for frame synchronization
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}
		ThrowIfFailed(m_Primarydevice->CreateFence(M_ShadowFence, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pShadowFence)));
		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.

	}
	InitMipmaps();

	ThrowIfFailed(m_Primarydevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, nullptr, IID_PPV_ARGS(&m_SetupCommandList)));
	CreateDepthStencil(m_width, m_height);
	m_SetupCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[0], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
	
}

void D3D12RHI::ExecSetUpList()
{
	ThrowIfFailed(m_SetupCommandList->Close());
	ExecList(m_SetupCommandList);
	WaitForGpu();
}

void D3D12RHI::ExecList(CommandListDef* list, bool IsFinal)
{
	if (IsFinal)
	{
		//PostFrame(list);
	}
	ID3D12CommandList* ppCommandLists[] = { list };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	ThrowIfFailed(m_commandQueue->Signal(pShadowFence, M_ShadowFence));	//set the value for exec complition
	HANDLE EventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	ThrowIfFailed(pShadowFence->SetEventOnCompletion(M_ShadowFence, EventHandle));//set the wait for the value complition
	WaitForSingleObject(EventHandle, INFINITE);//we need to wait to start the next queue!
	M_ShadowFence++;
	//change it!
	//m_commandQueue->Wait(pShadowFence, M_ShadowFence);
	//M_ShadowFence++;
}
void D3D12RHI::TransitionBuffers(bool In)
{
	m_SetupCommandList->Reset(m_commandAllocator, nullptr);
	
	if (In)
	{
		m_SetupCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	}
	else
	{
		m_SetupCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	}

	m_SetupCommandList->Close();
	ExecList(m_SetupCommandList);


}
void D3D12RHI::PresentFrame()
{
	//testing
	if (m_BudgetNotificationCookie == 1)
	{
		DisplayDeviceDebug();
		std::cout << "Memory Budget Changed" << std::endl;
	}
#if USEGPUTOGENMIPS
	if (count == 1)
	{
		MipmapShader->GenAllmips();
		
	}
#endif
	TransitionBuffers(false);
	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(0, 0));
	if (RequestedResize)
	{
		InternalResizeSwapChain(newwidth, newheight);
	}
	m_commandAllocator->Reset();
	//WaitForPreviousFrame();
	MoveToNextFrame();
	if (count == 0)
	{
		TransitionBuffers(true);
	}
	WaitForGpu();
	count++;
	if (count > 2)
	{
		HasSetup = true;
	}
	//WaitForGpu();
}

void D3D12RHI::ClearRenderTarget(ID3D12GraphicsCommandList* MainList)
{
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	MainList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	MainList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


}
void D3D12RHI::RenderToScreen(ID3D12GraphicsCommandList* list)
{
	list->RSSetViewports(1, &m_viewport);
	list->RSSetScissorRects(1, &m_scissorRect);
}

void D3D12RHI::PreFrameSetUp(ID3D12GraphicsCommandList* list, D3D12Shader* Shader)
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(Shader->GetCommandAllocator()->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(list->Reset(Shader->GetCommandAllocator(), Shader->GetPipelineShader()->m_pipelineState));

	// Set necessary state.
	list->SetGraphicsRootSignature(Shader->GetPipelineShader()->m_rootSignature);
}
void D3D12RHI::PreFrameSwap(ID3D12GraphicsCommandList* list)
{
	RenderToScreen(list);
	// Indicate that the back buffer will be used as a render target.
	//list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
	TransitionBuffers(true);
	SetScreenRenderTaget(list);

}
void D3D12RHI::SetScreenRenderTaget(ID3D12GraphicsCommandList* list)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	list->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
}

void D3D12RHI::PostFrame(ID3D12GraphicsCommandList* list)
{
	// Indicate that the back buffer will now be used to present.
	list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));

	ThrowIfFailed(list->Close());
}

void D3D12RHI::WaitForGpu()
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(m_commandQueue->Signal(m_fence, m_fenceValues[m_frameIndex]));

	// Wait until the fence has been processed.
	ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_fenceValues[m_frameIndex]++;
}

// Prepare to render the next frame.
void D3D12RHI::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
	ThrowIfFailed(m_commandQueue->Signal(m_fence, currentFenceValue));

	// Update the frame index.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}

ID3D12Device * D3D12RHI::GetDevice()
{
	if (Instance != nullptr)
	{
		return Instance->m_Primarydevice;
	}
	return nullptr;
}

void D3D12RHI::WaitForPreviousFrame()
{
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence, fence));
	m_fenceValue++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}


void GetHardwareAdapter(IDXGIFactory2 * pFactory, IDXGIAdapter1 ** ppAdapter)
{
	IDXGIAdapter1* adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter;
}
