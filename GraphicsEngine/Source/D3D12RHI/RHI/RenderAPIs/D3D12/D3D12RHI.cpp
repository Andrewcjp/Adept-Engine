#include "D3D12RHI.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Platform/Windows/WindowsWindow.h"
#include "D3D12Buffer.h"
#include "D3D12CommandList.h"
#include "D3D12DeviceContext.h"
#include "D3D12Framebuffer.h"
#include "D3D12InterGPUStagingResource.h"
#include "D3D12Query.h"
#include "GPUResource.h"
#include "OpenVR/headers/openvr.h"
#include "Raytracing/D3D12HighLevelAccelerationStructure.h"
#include "Raytracing/D3D12LowLevelAccelerationStructure.h"
#include "Raytracing/D3D12StateObject.h"
#include <dxgidebug.h>
#include <DXProgrammableCapture.h>  

static ConsoleVariable ForceGPUIndex("ForceDeviceIndex", -1, ECVarType::LaunchOnly, true);
static ConsoleVariable ForceSingleGPU("ForceSingleGPU", 0, ECVarType::LaunchOnly, false);
static ConsoleVariable ForceNoDebug("ForceNoDebug", 0, ECVarType::LaunchOnly, false);
static ConsoleVariable AllowWarp("AllowWarp", 0, ECVarType::LaunchOnly, false);
static ConsoleVariable EnableDred("EnableDred", 0, ECVarType::LaunchOnly, false);
D3D12RHI* D3D12RHI::Instance = nullptr;
D3D12RHI::D3D12RHI()
{
	Instance = this;
	//ForceGPUIndex.SetValue(1);
	//ForceSingleGPU.SetValue(true);
	//ForceNoDebug.SetValue(true);
	//AllowWarp.SetValue(true);
	EnableDred.SetValue(true);
}

D3D12RHI::~D3D12RHI()
{}
#if DRED
void D3D12RHI::RunDred()
{
	//dred is in windows 19h1
#if  NTDDI_WIN10_19H1

	HRESULT R = DeviceContexts[0]->GetDevice()->QueryInterface(IID_PPV_ARGS(&pDred));
	D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT DredAutoBreadcrumbsOutput;
	D3D12_DRED_PAGE_FAULT_OUTPUT DredPageFaultOutput;
	R = (pDred->GetAutoBreadcrumbsOutput(&DredAutoBreadcrumbsOutput));
	R = (pDred->GetPageFaultAllocationOutput(&DredPageFaultOutput));
	__debugbreak();
#endif
}
#endif

RHIQuery * D3D12RHI::CreateQuery(EGPUQueryType::Type type, DeviceContext * con)
{
	return new D3D12Query(type, con);
}

LowLevelAccelerationStructure* D3D12RHI::CreateLowLevelAccelerationStructure(DeviceContext * Device, const AccelerationStructureDesc & Desc)
{
	return new D3D12LowLevelAccelerationStructure(Device, Desc);
}

HighLevelAccelerationStructure* D3D12RHI::CreateHighLevelAccelerationStructure(DeviceContext * Device, const AccelerationStructureDesc & Desc)
{
	return new D3D12HighLevelAccelerationStructure(Device, Desc);
}

D3D12DeviceContext * D3D12RHI::DXConv(DeviceContext * D)
{
	return static_cast<D3D12DeviceContext*>(D);
}

D3D12Query * D3D12RHI::DXConv(RHIQuery * D)
{
	return static_cast<D3D12Query*>(D);
}

D3D12RHIUAV * D3D12RHI::DXConv(RHIUAV * D)
{
	return static_cast<D3D12RHIUAV*>(D);
}

D3D12Texture * D3D12RHI::DXConv(BaseTexture* D)
{
	return static_cast<D3D12Texture*>(D);
}

D3D12PipeLineStateObject * D3D12RHI::DXConv(RHIPipeLineStateObject * D)
{
	return static_cast<D3D12PipeLineStateObject*>(D);
}

D3D12Shader * D3D12RHI::DXConv(ShaderProgramBase * D)
{
	return static_cast<D3D12Shader*>(D);
}

D3D12FrameBuffer * D3D12RHI::DXConv(FrameBuffer * D)
{
	return static_cast<D3D12FrameBuffer*>(D);
}

D3D12Buffer * D3D12RHI::DXConv(RHIBuffer * D)
{
	return static_cast<D3D12Buffer*>(D);
}

D3D12CommandList * D3D12RHI::DXConv(RHICommandList * D)
{
	return static_cast<D3D12CommandList*>(D);
}

D3D12LowLevelAccelerationStructure * D3D12RHI::DXConv(LowLevelAccelerationStructure * D)
{
	return static_cast<D3D12LowLevelAccelerationStructure*>(D);
}

D3D12HighLevelAccelerationStructure * D3D12RHI::DXConv(HighLevelAccelerationStructure * D)
{
	return static_cast<D3D12HighLevelAccelerationStructure*>(D);
}

D3D12StateObject * D3D12RHI::DXConv(RHIStateObject * D)
{
	return static_cast<D3D12StateObject*>(D);
}

D3D12InterGPUStagingResource * D3D12RHI::DXConv(RHIInterGPUStagingResource * D)
{
	return static_cast<D3D12InterGPUStagingResource*>(D);
}


RHIRenderPass* D3D12RHI::CreateRenderPass(RHIRenderPassDesc & Desc, DeviceContext* Device)
{
	return new RHIRenderPass(Desc);
}

RHIInterGPUStagingResource* D3D12RHI::CreateInterGPUStagingResource(DeviceContext* Owner, const InterGPUDesc& desc)
{
	return new D3D12InterGPUStagingResource(Owner, desc);
}

RHIStateObject* D3D12RHI::CreateStateObject(DeviceContext* Device)
{
	return new D3D12StateObject(Device);
}

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
	SafeDelete(ScreenShotter);
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

void D3D12RHI::ReportDeviceData()
{
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (DeviceContexts[i] != nullptr)
		{
			//	DeviceContexts[i]->
		}
	}
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

std::string D3D12RHI::ReportMemory()
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
	std::string output = "";
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (DeviceContexts[i] != nullptr)
		{
			output.append("GPU" + std::to_string(i) + ": " + DeviceContexts[i]->GetMemoryReport() + " ");
		}
	}
	return output;
}

void D3D12RHI::LoadPipeLine()
{

#ifdef _DEBUG
#if !AFTERMATH
#define RUNDEBUG 1
#endif
#else 	
#define RUNDEBUG 0
#endif

	UINT dxgiFactoryFlags = 0;
#if RUNDEBUG //nsight needs this off
#if !_DEBUG
	Log::LogMessage("Validation layer running", Log::Warning);
#endif
	if (!ForceNoDebug.GetBoolValue() && !DetectGPUDebugger())
	{	//EnableShaderBasedValidation();

// Enable the debug layer (requires the Graphics Tools "optional feature").
// NOTE: Enabling the debug layer after device creation will invalidate the active device.
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			debugController->Release();
			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif
	if (EnableDred.GetBoolValue() && !ForceNoDebug.GetBoolValue())
	{
#if NTDDI_WIN10_19H1
		ID3D12DeviceRemovedExtendedDataSettings* pDredSettings;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&pDredSettings)));
		pDredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		pDredSettings->SetWatsonDumpEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		pDredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
#endif
	}

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
		Log::LogMessage("Found D3D12 GPU debugger, Warp adapter is now used instead of second physical GPU");
#endif
	}
	if (!FindAdaptors(factory, false))//Search adapters in a picky fashion 
	{
		Log::LogMessage("Failed to find select device index, Defaulting", Log::Severity::Warning);
		FindAdaptors(factory, true);//force find an adapter
	}
	//#DX12: handle 3 GPU
	if (GetSecondaryDevice() != nullptr)
	{
		GetPrimaryDevice()->LinkAdaptors(GetSecondaryDevice());
		GetSecondaryDevice()->LinkAdaptors(GetPrimaryDevice());
	}
#if RUNDEBUG
	if (!ForceNoDebug.GetBoolValue())
	{
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
					infoqueue[i]->AddMessage(D3D12_MESSAGE_CATEGORY::D3D12_MESSAGE_CATEGORY_INITIALIZATION, D3D12_MESSAGE_SEVERITY_WARNING, D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_LIVE_SAMPLER, "Init complete");
					infoqueue[i]->Release();
				}
			}
		}
	}
#endif
	DisplayDeviceDebug();
	Log::LogMessage(ReportMemory());

}

void D3D12RHI::HandleDeviceFailure()

{
	ensure(Instance);
	HRESULT HR;
	HR = Instance->GetPrimaryDevice()->GetDevice()->GetDeviceRemovedReason();
	ensureMsgf(HR == S_OK, +(std::string)D3D12Helpers::DXErrorCodeToString(HR));
#if DRED
	D3D12RHI::Get()->RunDred();
#endif
}

RHIPipeLineStateObject* D3D12RHI::CreatePSO(const RHIPipeLineStateDesc& Desc, DeviceContext * Device)
{
	D3D12PipeLineStateObject* NewObject = new D3D12PipeLineStateObject(Desc, Device);
	return NewObject;
}
#if ALLOW_RESOURCE_CAPTURE
void D3D12RHI::TriggerWriteBackResources()
{
	D3D12ReadBackCopyHelper::Get()->TriggerWriteBackAll();
}

#endif
RHIGPUSyncEvent* D3D12RHI::CreateSyncEvent(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue, DeviceContext * Device, DeviceContext * SignalDevice)
{
	return new D3D12GPUSyncEvent(WaitingQueue, SignalQueue, Device, SignalDevice);
}

void D3D12RHI::SubmitToVRComposter(FrameBuffer * fb, EEye::Type eye)
{
	//#dx12: check state is pixel shader resource 
	vr::VRTextureBounds_t bounds;
	bounds.uMin = 0.0f;
	bounds.uMax = 1.0f;
	bounds.vMin = 0.0f;
	bounds.vMax = 1.0f;

	vr::D3D12TextureData_t texture = {};
	texture.m_pResource = DXConv(fb)->GetResource(0)->GetResource();
	texture.m_pCommandQueue = DeviceContexts[0]->GetCommandQueueFromEnum(DeviceContextQueue::Graphics);
	vr::Texture_t leftEyeTexture = { (void *)&texture, vr::TextureType_DirectX12, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit((vr::Hmd_Eye)eye, &leftEyeTexture, &bounds, vr::Submit_Default);
}

void D3D12RHI::CreateSwapChainRTs()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	// Create a RTV for each frame.
	for (UINT n = 0; n < RHI::CPUFrameCount; n++)
	{
		ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_SwaprenderTargets[n])));
		GetDisplayDevice()->CreateRenderTargetView(m_SwaprenderTargets[n], nullptr, rtvHandle);
		m_RenderTargetResources[n] = new GPUResource(m_SwaprenderTargets[n], D3D12_RESOURCE_STATE_PRESENT, RHI::GetDefaultDevice());
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}
	NAME_D3D12_OBJECT(m_SwaprenderTargets[1]);
	NAME_D3D12_OBJECT(m_SwaprenderTargets[0]);
}

void D3D12RHI::ReleaseSwapRTs()
{
	for (UINT n = 0; n < RHI::CPUFrameCount; n++)
	{
		SafeDelete(m_RenderTargetResources[n]);
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
			SafeDelete(ScreenShotter);
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
#if AFTERMATH
#pragma optimize("",off)
void D3D12RHI::RunTheAfterMath()
{
	//	GFSDK_Aftermath_GetData()
	GFSDK_Aftermath_Device_Status state;
	GFSDK_Aftermath_GetDeviceStatus(&state);
	GFSDK_Aftermath_PageFaultInformation f;
	GFSDK_Aftermath_GetPageFaultInformation(&f);
	GFSDK_Aftermath_ContextData* data = new GFSDK_Aftermath_ContextData[handles.size()];
	GFSDK_Aftermath_GetData(handles.size(), handles.data(), data);
	__debugbreak();
}
#endif
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

	ScreenShotter = new D3D12ReadBackCopyHelper(RHI::GetDefaultDevice(), m_RenderTargetResources[0], true);
	ThrowIfFailed(GetDisplayDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, GetPrimaryDevice()->GetCommandAllocator(), nullptr, IID_PPV_ARGS(&m_SetupCommandList)));
	CreateDepthStencil(m_width, m_height);

	//if (RHI::GetDeviceCount() > 1)
	//{
	//	AsyncSync = (D3D12GPUSyncEvent*)RHI::CreateSyncEvent(DeviceContextQueue::InterCopy, DeviceContextQueue::InterCopy, RHI::GetDeviceContext(0), RHI::GetDeviceContext(1));
	//}
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
	if (Target == nullptr)
	{
		return;
	}
	for (UploadHeapStamped r : DeferredDeleteQueue)
	{
		LogEnsure(r.first != Target);
	}
	DeferredDeleteQueue.push_back(UploadHeapStamped(Target, RHI::GetFrameCount()));
}

D3D12RHI * D3D12RHI::Get()
{
	return Instance;
}

void D3D12RHI::PresentFrame()
{
	//todo: remove this
	if (m_RenderTargetResources[m_frameIndex]->GetCurrentState() != D3D12_RESOURCE_STATE_PRESENT)
	{
		m_SetupCommandList->Reset(GetPrimaryDevice()->GetCommandAllocator(), nullptr);
		m_RenderTargetResources[m_frameIndex]->SetResourceState(m_SetupCommandList, D3D12_RESOURCE_STATE_PRESENT);

		m_SetupCommandList->Close();
		GetPrimaryDevice()->ExecuteCommandList(m_SetupCommandList);
	}

	//only set up to grab the 0 frame of spawn chain
	if (RunScreenShot && m_frameIndex == 0)
	{
		ScreenShotter->WriteBackRenderTarget();
		//#DX12: should be an async task to write from GPU to disk also prevent capture before write is complete
		ScreenShotter->WriteToFile(AssetManager::DirectGetGeneratedDir());
		Log::LogMessage("Took ScreenShot");
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

#if LOG_RESOURCE_TRANSITIONS
	Log::LogMessage("-----Frame END------");
#endif
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
		if (index >= MAX_GPU_DEVICE_COUNT)
		{
			return nullptr;
		}
		return Instance->DeviceContexts[index];
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

bool D3D12RHI::FindAdaptors(IDXGIFactory2 * pFactory, bool ForceFind)
{
	int TargetIndex = ForceGPUIndex.GetIntValue();
	bool ForcingIndex = (TargetIndex != -1);
	if (ForceFind)
	{
		ForcingIndex = false;
	}
	IDXGIAdapter1* adapter;
	int CurrentDeviceIndex = 0;
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

#if BUILD_SHIPPING
		bool AllowSoft = false;
#else
		bool AllowSoft = AllowWarp.GetBoolValue();
#endif
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE && !AllowSoft)
		{
			if (TargetIndex == adapterIndex)
			{
				ForcingIndex = false;
			}
			adapter->Release();
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}
		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			if (ForcingIndex && adapterIndex != TargetIndex)
			{
				adapter->Release();
				continue;
			}
			D3D12DeviceContext** Device = nullptr;
			if (CurrentDeviceIndex >= MAX_GPU_DEVICE_COUNT)
			{
				return true;
			}
			Device = &DeviceContexts[CurrentDeviceIndex];
			if (*Device == nullptr)
			{
				*Device = new D3D12DeviceContext();
				//#SLI This needs to create more Devices 
				(*Device)->CreateDeviceFromAdaptor(adapter, CurrentDeviceIndex);
				CurrentDeviceIndex++;
				if (ForceSingleGPU.GetBoolValue())
				{
					Log::LogMessage("Forced Single Gpu Mode");
					return true;
				}
				if ((*Device)->IsPartOfNodeGroup())
				{
					//we have a linked group!
					//
					int ExtraNodes = glm::min(CurrentDeviceIndex + (*Device)->GetNodeCount() - 1, MAX_GPU_DEVICE_COUNT);
					for (int i = 0; i < ExtraNodes; i++)
					{
						DeviceContexts[CurrentDeviceIndex] = new D3D12DeviceContext();
						DeviceContexts[CurrentDeviceIndex]->CreateNodeDevice((*Device)->GetDevice(), i + 1, CurrentDeviceIndex);
						CurrentDeviceIndex++;
					}
				}
			}
		}
	}
	return (CurrentDeviceIndex != 0);
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

BaseTexture * D3D12RHI::CreateTexture(const RHITextureDesc& Desc, DeviceContext* Device)
{
	if (Device == nullptr)
	{
		Device = D3D12RHI::GetDefaultDevice();
	}
	return new D3D12Texture(Device);
}


FrameBuffer* D3D12RHI::CreateFrameBuffer(DeviceContext* Device, const RHIFrameBufferDesc& Desc)
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
#if !BUILD_SHIPPING
	D3D12Shader::PrintShaderStats();
#endif
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

