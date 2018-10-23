#include "stdafx.h"
#include "D3D12RHI.h"
#include "RHI/RHI.h"
#include "RHI/RHI_inc.h"
#include "RHI/RHITypes.h"
#include "include\glm\gtx\transform.hpp"
#include "GPUResource.h"
#include "D3D12TimeManager.h"
#include "Core/Platform/PlatformCore.h"
#include "Rendering/Core/Mesh.h"
#include "Rendering/Core/Mesh.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Assets/ImageIO.h"
#include "RHI/RenderAPIs/D3D12/D3D12Texture.h"
#include "RHI/RenderAPIs/D3D12/D3D12Shader.h"
#include "RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include "RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#include "D3D12DeviceContext.h"
#include <DXProgrammableCapture.h>  
#include <dxgidebug.h>
#include <D3Dcompiler.h>
#include "Core/Platform/ConsoleVariable.h"
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "d3dcompiler.lib")
#include "Core/Platform/Windows/WindowsWindow.h"
static ConsoleVariable ForceSingleGPU("ForceSingleGPU", 0, ECVarType::LaunchOnly);
D3D12RHI* D3D12RHI::Instance = nullptr;
D3D12RHI::D3D12RHI()
{
	Instance = this;
}

D3D12RHI::~D3D12RHI()
{}

void D3D12RHI::DestroyContext()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	ReleaseUploadHeaps(true);
	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, nullptr);
	}
	ReleaseSwapRTs();
	SafeRelease(m_swapChain);
	MemoryUtils::DeleteCArray(DeviceContexts, MAX_GPU_DEVICE_COUNT);
	SafeRelease(m_rtvHeap);
	SafeRelease(m_dsvHeap);
	SafeRelease(m_SetupCommandList);
	SafeRelease(factory);
	delete ScreenShotter;
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
	FeatureData.NumFeatureLevels = 4;
	HRESULT hr = pDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &FeatureData, sizeof(FeatureData));
	if (SUCCEEDED(hr))
	{
		return FeatureData.MaxSupportedFeatureLevel;
	}
	return D3D_FEATURE_LEVEL_11_0;
}

bool D3D12RHI::DetectGPUDebugger()
{
	IDXGraphicsAnalysis* pGraphicsAnalysis;
	HRESULT getAnalysis = DXGIGetDebugInterface1(0, __uuidof(pGraphicsAnalysis), reinterpret_cast<void**>(&pGraphicsAnalysis));
	if (getAnalysis != S_OK)
	{
		return false;
	}
	return true;
}
void D3D12RHI::WaitForGPU()
{
	WaitForAllGPUS();
}

void D3D12RHI::DisplayDeviceDebug()
{
	//Log::OutS << "Primary Adaptor Has " << GetMemory() << Log::OutS;1
}

std::string D3D12RHI::GetMemory()
{
	if (RHI::GetFrameCount() % 60 == 0 || !HasSetup)
	{
		for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
		{
			if (DeviceContexts[i] != nullptr)
			{
				DeviceContexts[i]->SampleVideoMemoryInfo();
			}
		}
	}
	std::string output = GetPrimaryDevice()->GetMemoryReport();

	if (GetSecondaryDevice() != nullptr)
	{
		output.append(" Sec:");
		output.append(GetSecondaryDevice()->GetMemoryReport());
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

	UINT dxgiFactoryFlags = 0;
#if RUNDEBUG //nsight needs this off
	//EnableShaderBasedValidation();

	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
		debugController->Release();
		// Enable additional debug layers.
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));
	ReportObjects();

	if (DetectGPUDebugger())
	{
#if 1
		ForceSingleGPU.SetValue(true);
#else
		IDXGIAdapter* warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		DeviceContexts[1] = new D3D12DeviceContext();
		DeviceContexts[1]->CreateDeviceFromAdaptor((IDXGIAdapter1*)warpAdapter, 1);
		Log::LogMessage("Found D3D12 GPU debugger, Warp adaptor is now used instead of second physical GPU");
#endif
	}
	FindAdaptors(factory);
	//todo: handle 3 GPU
	if (GetSecondaryDevice() != nullptr)
	{
		GetPrimaryDevice()->LinkAdaptors(GetSecondaryDevice());
		GetSecondaryDevice()->LinkAdaptors(GetPrimaryDevice());
	}
#if RUNDEBUG
	ID3D12InfoQueue* infoqueue[MAX_GPU_DEVICE_COUNT] = { nullptr };
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (DeviceContexts[i] != nullptr)
		{
			DeviceContexts[i]->GetDevice()->QueryInterface(IID_PPV_ARGS(&infoqueue[i]));
			if (infoqueue[i] != nullptr)
			{
				infoqueue[i]->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
				infoqueue[i]->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
				infoqueue[i]->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
				infoqueue[i]->Release();
			}
		}
	}
#endif
	DisplayDeviceDebug();
}

void D3D12RHI::HandleDeviceFailure()
{
	ensure(Instance);
	HRESULT HR;
	HR = Instance->GetPrimaryDevice()->GetDevice()->GetDeviceRemovedReason();
	ensureMsgf(HR == S_OK, +(std::string)D3D12Helpers::DXErrorCodeToString(HR));
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
			GetDisplayDevice()->CreateRenderTargetView(m_SwaprenderTargets[n], nullptr, rtvHandle);
			m_RenderTargetResources[n] = new GPUResource(m_SwaprenderTargets[n], D3D12_RESOURCE_STATE_PRESENT);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
		NAME_D3D12_OBJECT(m_SwaprenderTargets[1]);
		NAME_D3D12_OBJECT(m_SwaprenderTargets[0]);

	}
}

void D3D12RHI::ReleaseSwapRTs()
{
	for (UINT n = 0; n < RHI::CPUFrameCount; n++)
	{
		delete m_RenderTargetResources[n];
	}
	SafeRelease(m_depthStencil);
}

void D3D12RHI::ResizeSwapChain(int x, int y)
{
	if (x == 0 || y == 0)
	{
		return;
	}
	GetPrimaryDevice()->WaitForGpu();
	if (m_swapChain != nullptr)
	{
		ReleaseSwapRTs();
		if (ScreenShotter != nullptr)
		{
			delete ScreenShotter;
		}
		ThrowIfFailed(m_swapChain->ResizeBuffers(RHI::CPUFrameCount, x, y, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
		CreateSwapChainRTs();
		m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(x), static_cast<float>(y));
		m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(x), static_cast<LONG>(y));
		CreateDepthStencil(x, y);
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
		ScreenShotter = new D3D12ReadBackCopyHelper(RHI::GetDefaultDevice(), m_RenderTargetResources[0]);
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

	ThrowIfFailed(GetDisplayDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&m_depthStencil)
	));

	NAME_D3D12_OBJECT(m_depthStencil);

	GetDisplayDevice()->CreateDepthStencilView(m_depthStencil, &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void D3D12RHI::InitSwapChain()
{
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = RHI::CPUFrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	IDXGISwapChain1* swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		GetPrimaryDevice()->GetCommandQueue(),		// Swap chain needs the queue so that it can force a flush on it.
		PlatformWindow::GetHWND(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	m_swapChain = (IDXGISwapChain3*)swapChain;
	// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(PlatformWindow::GetHWND(), DXGI_MWA_NO_ALT_ENTER));

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	m_swapChain->SetFullscreenState(false, nullptr);

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = RHI::CPUFrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(GetDisplayDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

		m_rtvDescriptorSize = GetDisplayDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(GetDisplayDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
		m_rtvDescriptorSize = GetDisplayDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	}
	SafeRelease(factory);
	CreateSwapChainRTs();

	ScreenShotter = new D3D12ReadBackCopyHelper(RHI::GetDefaultDevice(), m_RenderTargetResources[0]);
	ThrowIfFailed(GetDisplayDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, GetPrimaryDevice()->GetCommandAllocator(), nullptr, IID_PPV_ARGS(&m_SetupCommandList)));
	CreateDepthStencil(m_width, m_height);
}

void D3D12RHI::SetFullScreenState(bool state)
{
	IsFullScreen = state;
	if (m_swapChain != nullptr)
	{
		m_swapChain->SetFullscreenState(state, nullptr);
	}
}

void D3D12RHI::WaitForAllGPUS()
{
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (DeviceContexts[i] != nullptr)
		{
			DeviceContexts[i]->CPUWaitForAll();
		}
	}
}

void D3D12RHI::ResetAllGPUCopyEngines()
{
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (DeviceContexts[i] != nullptr)
		{
			DeviceContexts[i]->ResetCopyEngine();
		}
	}
}

void D3D12RHI::UpdateAllCopyEngines()
{
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (DeviceContexts[i] != nullptr)
		{
			DeviceContexts[i]->UpdateCopyEngine();
		}
	}
}

void D3D12RHI::ExecSetUpList()
{
	ThrowIfFailed(m_SetupCommandList->Close());
	GetPrimaryDevice()->ExecuteCommandList(m_SetupCommandList);
	UpdateAllCopyEngines();
	WaitForAllGPUS();
	ReleaseUploadHeaps();
	ResetAllGPUCopyEngines();
}

void D3D12RHI::ReleaseUploadHeaps(bool force)
{
	//The Visual studio Graphics Debugger Causes a crash here in Driver Code. Cause Unknown
	if (!DetectGPUDebugger())
	{
		for (int i = (int)DeferredDeleteQueue.size() - 1; i >= 0; i--)
		{
			const int CurrentFrame = RHI::GetFrameCount();
			if (DeferredDeleteQueue[i].second + RHI::CPUFrameCount < CurrentFrame || force)
			{
				SafeRelease(DeferredDeleteQueue[i].first);
				DeferredDeleteQueue.erase(DeferredDeleteQueue.begin() + i);
			}
		}
	}
}

void D3D12RHI::AddObjectToDeferredDeleteQueue(IUnknown* Target)
{
	for (UploadHeapStamped r : DeferredDeleteQueue)
	{
		ensure(r.first != Target);
	}
	DeferredDeleteQueue.push_back(UploadHeapStamped(Target, RHI::GetFrameCount()));
}

D3D12RHI * D3D12RHI::Get()
{
	return Instance;
}

void D3D12RHI::PresentFrame()
{
	if (m_RenderTargetResources[m_frameIndex]->GetCurrentState() != D3D12_RESOURCE_STATE_PRESENT)
	{
		m_SetupCommandList->Reset(GetPrimaryDevice()->GetCommandAllocator(), nullptr);
		((D3D12TimeManager*)GetPrimaryDevice()->GetTimeManager())->EndTotalGPUTimer(m_SetupCommandList);
		m_RenderTargetResources[m_frameIndex]->SetResourceState(m_SetupCommandList, D3D12_RESOURCE_STATE_PRESENT);

		m_SetupCommandList->Close();
		GetPrimaryDevice()->ExecuteCommandList(m_SetupCommandList);
	}

	//only set up to grab the 0 frame of spawn chain
	if (RunScreenShot && m_frameIndex == 0)
	{
		ScreenShotter->WriteBackRenderTarget();
		ScreenShotter->WriteToFile(AssetPathRef("Saved\\Screen"));
		RunScreenShot = false;
	}
	ThrowIfFailed(m_swapChain->Present(0, 0));
	if (!RHI::AllowCPUAhead())
	{
		WaitForAllGPUS();
	}

	UpdateAllCopyEngines();
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (DeviceContexts[i] != nullptr)
		{
			DeviceContexts[i]->MoveNextFrame(m_swapChain->GetCurrentBackBufferIndex());
		}
	}

	//all execution this frame has finished 
	//so all resources should be in the correct state!	
	ReleaseUploadHeaps();
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (DeviceContexts[i] != nullptr)
		{
			DeviceContexts[i]->ResetDeviceAtEndOfFrame();
		}
	}
	if (RHI::GetFrameCount() > 2)
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

void D3D12RHI::SetScreenRenderTarget(ID3D12GraphicsCommandList* list)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	list->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	m_RenderTargetResources[m_frameIndex]->SetResourceState(list, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

DeviceContext * D3D12RHI::GetDeviceContext(int index)
{
	if (Instance != nullptr)
	{
		if (index == 0)
		{
			return Instance->DeviceContexts[0];
		}
		else if (index == 1)
		{
			return Instance->DeviceContexts[1];
		}
	}
	return nullptr;
}

DeviceContext * D3D12RHI::GetDefaultDevice()
{
	if (Instance != nullptr)
	{
		return Instance->GetPrimaryDevice();
	}
	return nullptr;
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
			adapter->Release();
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}
		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			D3D12DeviceContext** Device = nullptr;
			if (CurrentDeviceIndex >= MAX_GPU_DEVICE_COUNT)
			{
				return;
			}
			Device = &DeviceContexts[CurrentDeviceIndex];
			if (*Device == nullptr)
			{
				*Device = new D3D12DeviceContext();
				(*Device)->CreateDeviceFromAdaptor(adapter, CurrentDeviceIndex);
				CurrentDeviceIndex++;
				if (ForceSingleGPU.GetBoolValue())
				{
					Log::LogMessage("Forced Single Gpu Mode");
					return;
				}
			}

		}
	}
}

RHIBuffer * D3D12RHI::CreateRHIBuffer(ERHIBufferType::Type type, DeviceContext* Device)
{
	return new D3D12Buffer(type, Device);
}

RHIUAV * D3D12RHI::CreateUAV(DeviceContext * Device)
{
	return new D3D12RHIUAV(Device);
}

RHICommandList * D3D12RHI::CreateCommandList(ECommandListType::Type Type, DeviceContext * Device)
{
	if (Device == nullptr)
	{
		Device = D3D12RHI::GetDefaultDevice();
	}
	return new D3D12CommandList(Device, Type);
}

void D3D12RHI::TriggerBackBufferScreenShot()
{
	RunScreenShot = true;
}

BaseTexture * D3D12RHI::CreateTexture(DeviceContext* Device)
{
	if (Device == nullptr)
	{
		Device = D3D12RHI::GetDefaultDevice();
	}
	return new D3D12Texture(Device);
}


FrameBuffer * D3D12RHI::CreateFrameBuffer(DeviceContext * Device, RHIFrameBufferDesc & Desc)
{
	if (Device == nullptr)
	{
		Device = D3D12RHI::GetDefaultDevice();
	}
	return new D3D12FrameBuffer(Device, Desc);
}

ShaderProgramBase * D3D12RHI::CreateShaderProgam(DeviceContext* Device)
{
	if (Device == nullptr)
	{
		Device = D3D12RHI::GetDefaultDevice();
	}
	return new D3D12Shader(Device);
}

bool D3D12RHI::InitWindow(int w, int h)
{
	m_width = w;
	m_height = h;
	m_aspectRatio = static_cast<float>(w) / static_cast<float>(h);
	InitSwapChain();
	return false;
}

bool D3D12RHI::InitRHI()
{
	LoadPipeLine();
	return false;
}

bool D3D12RHI::DestoryRHI()
{
	DestroyContext();
	return false;
}

void D3D12RHI::RHISwapBuffers()
{
	PresentFrame();
}

void D3D12RHI::RHIRunFirstFrame()
{
	ExecSetUpList();
}

D3D12DeviceContext * D3D12RHI::GetPrimaryDevice()
{
	return DeviceContexts[0];
}

D3D12DeviceContext * D3D12RHI::GetSecondaryDevice()
{
#if (MAX_GPU_DEVICE_COUNT > 1)
	return DeviceContexts[1];
#else 
	return nullptr;
#endif
}

D3D12DeviceContext * D3D12RHI::GetThridDevice()
{
#if (MAX_GPU_DEVICE_COUNT > 2)
	return DeviceContexts[2];
#else 
	return nullptr;
#endif
}

ID3D12Device * D3D12RHI::GetDisplayDevice()
{
	return GetPrimaryDevice()->GetDevice();
}

RHITextureArray * D3D12RHI::CreateTextureArray(DeviceContext* Device, int Length)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}
	return new D3D12RHITextureArray(Device, Length);;
}


class D3D12RHIModule : public RHIModule
{
	virtual RHIClass* GetRHIClass()
	{
		return new D3D12RHI();
	}
};

#ifdef D3D12RHI_EXPORT
IMPLEMENT_MODULE_DYNAMIC(D3D12RHIModule);
#endif

