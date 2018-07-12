#include "stdafx.h"
#include "D3D12RHI.h"
#include "BaseApplication.h"
#include <D3Dcompiler.h>
#include "glm\glm.hpp"
#include "include\glm\gtx\transform.hpp"
#include "Rendering/Shaders/ShaderMipMap.h"
#include "GPUResource.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHI.h"
#include "D3D12TimeManager.h"
#include <dxgidebug.h>
D3D12RHI* D3D12RHI::Instance = nullptr;
D3D12RHI::D3D12RHI()
	:m_fenceValues{}
{
	Instance = this;
}

D3D12RHI::~D3D12RHI()
{}

void D3D12RHI::InitContext()
{}

void D3D12RHI::DestroyContext()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForGpu();
	ReleaseSwapRTs();
	delete MipmapShader;
	GetCommandQueue()->Release();
	delete PrimaryDevice;
	if (SecondaryDevice)
	{
		delete SecondaryDevice;
	}
	m_rtvHeap->Release();
	m_dsvHeap->Release();
	m_SetupCommandList->Release();
	CloseHandle(m_fenceEvent);
	m_fence->Release();
	ReportObjects();
}

void EnableShaderBasedValidation()
{
	ID3D12Debug* spDebugController0;
	ID3D12Debug1* spDebugController1;
	(D3D12GetDebugInterface(IID_PPV_ARGS(&spDebugController0)));
	(spDebugController0->QueryInterface(IID_PPV_ARGS(&spDebugController1)));
	spDebugController1->SetEnableGPUBasedValidation(true);
}

void D3D12RHI::CheckFeatures(ID3D12Device* pDevice)
{
	D3D12_FEATURE_DATA_D3D12_OPTIONS FeatureData;
	ZeroMemory(&FeatureData, sizeof(FeatureData));
	HRESULT hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &FeatureData, sizeof(FeatureData));
	if (SUCCEEDED(hr))
	{
		//check(FeatureData.CrossAdapterRowMajorTextureSupported);
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

void D3D12RHI::ReportObjects()
{
#ifdef _DEBUG
	IDXGIDebug* pDXGIDebug;
	typedef HRESULT(__stdcall *fPtr)(const IID&, void**);
	HMODULE hDll = GetModuleHandleW(L"dxgidebug.dll");
	fPtr DXGIGetDebugInterface = (fPtr)GetProcAddress(hDll, "DXGIGetDebugInterface");
	if (DXGIGetDebugInterface)
	{
		DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&pDXGIDebug);
		if (pDXGIDebug)
		{
			pDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		}
	}
#endif
	//DXGIGetDebugInterface(__uuidof(IDXGIInfoQueue), (void**)&pIDXGIInfoQueue);//DXGI_DEBUG_RLO_IGNORE_INTERNAL
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

void D3D12RHI::WaitForAllDevices()
{
	PrimaryDevice->CPUWaitForAll();
	if (SecondaryDevice != nullptr)
	{
		SecondaryDevice->CPUWaitForAll();
	}
}

void D3D12RHI::DisplayDeviceDebug()
{
	Log::OutS  << "Primary Adaptor Has " << GetMemory() << Log::OutS;
}

std::string D3D12RHI::GetMemory()
{
	if (PerfCounter > 60 || !HasSetup)
	{
		PerfCounter = 0;
		PrimaryDevice->SampleVideoMemoryInfo();
		if (SecondaryDevice != nullptr)
		{
			SecondaryDevice->SampleVideoMemoryInfo();
		}
	}
	std::string output = PrimaryDevice->GetMemoryReport();

	if (SecondaryDevice != nullptr)
	{
		output.append(" Sec:");
		output.append(SecondaryDevice->GetMemoryReport());
	}
	return output;
}

void D3D12RHI::LoadPipeLine()
{
#ifdef _DEBUG
#define RUNDEBUG 1
#else 
#define RUNDEBUG 0
#endif
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	UINT dxgiFactoryFlags = 0;
#if RUNDEBUG //nsight needs this off
	//EnableShaderBasedValidation();

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
	ReportObjects();
#if 0
	IDXGIAdapter* warpAdapter;
	ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
	SecondaryDevice = new DeviceContext();
	SecondaryDevice->CreateDeviceFromAdaptor((IDXGIAdapter1*)warpAdapter, 1);
#endif

	FindAdaptors(factory);
	if (SecondaryDevice != nullptr)
	{
		PrimaryDevice->LinkAdaptors(SecondaryDevice);
		SecondaryDevice->LinkAdaptors(PrimaryDevice);

	}
#if RUNDEBUG
	ID3D12InfoQueue* infoqueue[MAX_DEVICE_COUNT] = {nullptr};
	PrimaryDevice->GetDevice()->QueryInterface(IID_PPV_ARGS(&infoqueue[0]));
	if (SecondaryDevice != nullptr)
	{
		SecondaryDevice->GetDevice()->QueryInterface(IID_PPV_ARGS(&infoqueue[1]));
	}
	for (int i = 0; i < MAX_DEVICE_COUNT; i++)
	{
		if (infoqueue[i] != nullptr)
		{
			infoqueue[i]->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			infoqueue[i]->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			infoqueue[i]->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
		}
	}
#endif

	DisplayDeviceDebug();
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = RHI::CPUFrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	CheckFeatures(GetDevice());
	IDXGISwapChain1* swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		GetCommandQueue(),		// Swap chain needs the queue so that it can force a flush on it.
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
	m_swapChain->SetFullscreenState(IsFullScreen,nullptr);
	//m_swapChain->ResizeBuffers()
	//InternalResizeSwapChain();
	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = RHI::CPUFrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

		m_rtvDescriptorSize = GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
		m_rtvDescriptorSize = GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	}
	factory->Release();
	CreateSwapChainRTs();
}


void D3D12RHI::CreateSwapChainRTs()
{
	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < RHI::CPUFrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_SwaprenderTargets[n])));
			GetDevice()->CreateRenderTargetView(m_SwaprenderTargets[n], nullptr, rtvHandle);
			m_RenderTargetResources[n] = new GPUResource(m_SwaprenderTargets[n], D3D12_RESOURCE_STATE_PRESENT);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
		NAME_D3D12_OBJECT(m_SwaprenderTargets[1]);
		NAME_D3D12_OBJECT(m_SwaprenderTargets[0]);

	}
}

void D3D12RHI::InitMipmaps()
{
#if USEGPUTOGENMIPS_ATRUNTIME
	MipmapShader = new ShaderMipMap();
#endif
}

void D3D12RHI::InternalResizeSwapChain(int x, int y)
{
	if (m_swapChain != nullptr)
	{
		ReleaseSwapRTs();
		for (UINT n = 0; n < RHI::CPUFrameCount; n++)
		{
			m_fenceValues[n] = m_fenceValues[m_frameIndex];
		}
		ThrowIfFailed(m_swapChain->ResizeBuffers(RHI::CPUFrameCount, x, y, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
		CreateSwapChainRTs();
		m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(x), static_cast<float>(y));
		m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(x), static_cast<LONG>(y));
		CreateDepthStencil(x, y);
		for (int i = 0; i < FrameBuffersLinkedToSwapChain.size(); i++)
		{
			FrameBuffersLinkedToSwapChain[i]->Resize(x, y);
		}
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
		RequestedResize = false;
	}
}

void D3D12RHI::ReleaseSwapRTs()
{
	for (UINT n = 0; n < RHI::CPUFrameCount; n++)
	{
		m_SwaprenderTargets[n]->Release();
		delete m_RenderTargetResources[n];
	}
	m_depthStencil->Release();
}

void D3D12RHI::ResizeSwapChain(int x, int y, bool Force)
{
	if (x == 0 || y == 0)
	{
		return;
	}
	if (m_swapChain != nullptr)
	{
		RequestedResize = true;
		newwidth = x;
		newheight = y;
	}
	if (Force)
	{
		PrimaryDevice->WaitForGpu();
		InternalResizeSwapChain(x, y);
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

	ThrowIfFailed(GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&m_depthStencil)
	));

	NAME_D3D12_OBJECT(m_depthStencil);

	GetDevice()->CreateDepthStencilView(m_depthStencil, &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void D3D12RHI::LoadAssets()
{
	ThrowIfFailed(GetDevice()->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValues[m_frameIndex] = 1;
	// Create an event handle to use for frame synchronization
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
	ThrowIfFailed(GetDevice()->CreateFence(M_ShadowFence, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pShadowFence)));
	InitMipmaps();

	ThrowIfFailed(GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, PrimaryDevice->GetCommandAllocator(), nullptr, IID_PPV_ARGS(&m_SetupCommandList)));
	CreateDepthStencil(m_width, m_height);
}

void D3D12RHI::ToggleFullScreenState()
{
	IsFullScreen = !IsFullScreen;
	m_swapChain->SetFullscreenState(IsFullScreen, nullptr);
}

void D3D12RHI::ExecSetUpList()
{
	ThrowIfFailed(m_SetupCommandList->Close());
	ExecList(m_SetupCommandList);//todo: move this!
	PrimaryDevice->UpdateCopyEngine();
	if (SecondaryDevice != nullptr)
	{
		SecondaryDevice->UpdateCopyEngine();
	}
	WaitForGpu();
	ReleaseUploadHeap();
}

void D3D12RHI::ReleaseUploadHeap()
{
	if (CPUAheadCount < RHI::CPUFrameCount)
	{
		//ensure the heap are not still in use!
		return;
	}
	for (int i = 0; i < UsedUploadHeaps.size(); i++)
	{
		UsedUploadHeaps[i]->Release();
	}
	UsedUploadHeaps.clear();
}

void D3D12RHI::AddUploadToUsed(ID3D12Resource* Target)
{
	UsedUploadHeaps.push_back(Target);
}

void D3D12RHI::ExecList(ID3D12GraphicsCommandList* list, bool Block)
{
	ID3D12CommandList* ppCommandLists[] = { list };
	GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void D3D12RHI::PresentFrame()
{
	PerfCounter++;
	if (m_RenderTargetResources[m_frameIndex]->GetCurrentState() != D3D12_RESOURCE_STATE_PRESENT)
	{
		m_SetupCommandList->Reset(PrimaryDevice->GetCommandAllocator(), nullptr);
		PrimaryDevice->GetTimeManager()->EndTotalGPUTimer(m_SetupCommandList);
		m_RenderTargetResources[m_frameIndex]->SetResourceState(m_SetupCommandList, D3D12_RESOURCE_STATE_PRESENT);

		m_SetupCommandList->Close();
		PrimaryDevice->ExecuteCommandList(m_SetupCommandList);
	}

	if (m_BudgetNotificationCookie == 1)
	{
		DisplayDeviceDebug();
		Log::OutS  << "Memory Budget Changed" << Log::OutS;
	}
#if USEGPUTOGENMIPS_ATRUNTIME
	if (count == 1)
	{
		//PrimaryDevice->WaitForGpu();
		//MipmapShader->GenAllmips(100);
	}
#endif

	ThrowIfFailed(m_swapChain->Present(0, 0));
	if (!RHI::AllowCPUAhead())
	{
		PrimaryDevice->WaitForGpu();
		if (SecondaryDevice != nullptr)
		{
			SecondaryDevice->WaitForGpu();
		}
	}

	MoveToNextFrame();
	PrimaryDevice->CurrentFrameIndex = m_frameIndex;
	if (SecondaryDevice != nullptr)
	{
		SecondaryDevice->CurrentFrameIndex = m_frameIndex;
	}
	//all execution this frame has finished 
	//so all resources should be in the correct state!
	PrimaryDevice->UpdateCopyEngine();
	if (SecondaryDevice != nullptr)
	{
		SecondaryDevice->UpdateCopyEngine();
	}
	PrimaryDevice->ResetDeviceAtEndOfFrame();
	if (SecondaryDevice != nullptr)
	{
		SecondaryDevice->ResetDeviceAtEndOfFrame();
	}
	CPUAheadCount++;
	if (CPUAheadCount > 2)
	{
		HasSetup = true;
	}
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
	SetScreenRenderTarget(list);
}

void D3D12RHI::SetScreenRenderTarget(ID3D12GraphicsCommandList* list)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	list->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	m_RenderTargetResources[m_frameIndex]->SetResourceState(list, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void D3D12RHI::PostFrame(ID3D12GraphicsCommandList* list)
{
	// Indicate that the back buffer will now be used to present.
	//list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));

	ThrowIfFailed(list->Close());
}

void D3D12RHI::WaitForGpu()
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(GetCommandQueue()->Signal(m_fence, m_fenceValues[m_frameIndex]));

	// Wait until the fence has been processed.
	ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));

	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_fenceValues[m_frameIndex] += 1;
}

// Prepare to render the next frame.
void D3D12RHI::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
	ThrowIfFailed(GetCommandQueue()->Signal(m_fence, currentFenceValue));
	// Update the frame index.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	const UINT64 value = m_fence->GetCompletedValue();
	if (value < m_fenceValues[m_frameIndex])
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
		return Instance->PrimaryDevice->GetDevice();
	}
	return nullptr;
}

ID3D12CommandQueue * D3D12RHI::GetCommandQueue()
{
	return PrimaryDevice->GetCommandQueue();
}

DeviceContext * D3D12RHI::GetDeviceContext(int index)
{
	if (Instance != nullptr)
	{
		if (index == 0)
		{
			return Instance->PrimaryDevice;
		}
		else if (index == 1)
		{
			return Instance->SecondaryDevice;
		}
	}
	return nullptr;
}

DeviceContext * D3D12RHI::GetDefaultDevice()
{
	if (Instance != nullptr)
	{
		return Instance->PrimaryDevice;
	}
	return nullptr;
}

void D3D12RHI::AddLinkedFrameBuffer(FrameBuffer * target)
{
	ensure(target != nullptr);
	FrameBuffersLinkedToSwapChain.push_back(target);
}

void D3D12RHI::WaitForPreviousFrame()
{
	//// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	//// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	//// sample illustrates how to use fences for efficient resource usage and to
	//// maximize GPU utilization.

	//// Signal and increment the fence value.
	//const UINT64 fence = m_fenceValue;
	//ThrowIfFailed(GetCommandQueue()->Signal(m_fence, fence));
	//m_fenceValue++;

	//// Wait until the previous frame is finished.
	//if (m_fence->GetCompletedValue() < fence)
	//{
	//	ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
	//	WaitForSingleObject(m_fenceEvent, INFINITE);
	//}

	//m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}
void D3D12RHI::FindAdaptors(IDXGIFactory2 * pFactory)
{
	IDXGIAdapter1* adapter;
	int CurrentDeviceIndex = 0;
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
			DeviceContext** Device = nullptr;
			if (CurrentDeviceIndex == 0)
			{
				Device = &PrimaryDevice;
			}
			else if (CurrentDeviceIndex == 1)
			{
				Device = &SecondaryDevice;
			}
			else
			{
				return;
			}

			if (*Device == nullptr)
			{
				*Device = new DeviceContext();
				(*Device)->CreateDeviceFromAdaptor(adapter, CurrentDeviceIndex);
				CurrentDeviceIndex++;
			}
		}
	}

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
