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
#if SUPPORT_OPENVR 
#include "OpenVR/headers/openvr.h"
#endif
#include "Raytracing/D3D12HighLevelAccelerationStructure.h"
#include "Raytracing/D3D12LowLevelAccelerationStructure.h"
#include "Raytracing/D3D12StateObject.h"
#if SUPPORT_DXGI
#include <dxgidebug.h>
#include <DXProgrammableCapture.h>  
#endif
#include "DescriptorCache.h"
#include "DXMemoryManager.h"
#include "D3D12RHITexture.h"
#include "RHI/RHITexture.h"
#include "DXFence.h"


static ConsoleVariable ForceNoDebug("ForceNoDebug", 0, ECVarType::LaunchOnly, false);

static ConsoleVariable EnableDred("EnableDred", 0, ECVarType::LaunchOnly, false);
D3D12RHI* D3D12RHI::Instance = nullptr;
D3D12RHI::D3D12RHI()
{
	Instance = this;
	//ForceGPUIndex.SetValue(1);
//	ForceSingleGPU.SetValue(true);
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
#if  WIN10_1903
	HRESULT R = DeviceContexts[0]->GetDevice()->QueryInterface(ID_PASS(&pDred));
	D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT DredAutoBreadcrumbsOutput;
	D3D12_DRED_PAGE_FAULT_OUTPUT DredPageFaultOutput;
	R = (pDred->GetAutoBreadcrumbsOutput(&DredAutoBreadcrumbsOutput));
	R = (pDred->GetPageFaultAllocationOutput(&DredPageFaultOutput));
	ensureFatalMsgf(R == S_OK, "");
#endif
}
#endif

RHIQuery * D3D12RHI::CreateQuery(EGPUQueryType::Type type, DeviceContext * con)
{
	return new D3D12Query(type, con);
}

D3D12RHITexture * D3D12RHI::DXConv(RHITexture * D)
{
	return static_cast<D3D12RHITexture*>(D);
}
D3D12DeviceContext * D3D12RHI::DXConv(DeviceContext * D)
{
	return static_cast<D3D12DeviceContext*>(D);
}

D3D12Query * D3D12RHI::DXConv(RHIQuery * D)
{
	return static_cast<D3D12Query*>(D);
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
#if WIN10_1809
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
#endif
D3D12InterGPUStagingResource * D3D12RHI::DXConv(RHIInterGPUStagingResource * D)
{
	return static_cast<D3D12InterGPUStagingResource*>(D);
}

D3D12RHITextureArray * D3D12RHI::DXConv(RHITextureArray * D)
{
	return static_cast<D3D12RHITextureArray*>(D);
}

RHIRenderPass* D3D12RHI::CreateRenderPass(const RHIRenderPassDesc & Desc, DeviceContext* Device)
{
	return new RHIRenderPass(Desc);
}

RHIInterGPUStagingResource* D3D12RHI::CreateInterGPUStagingResource(DeviceContext* Owner, const InterGPUDesc& desc)
{
	return new D3D12InterGPUStagingResource(Owner, desc);
}

RHITexture* D3D12RHI::CreateTexture2()
{
	return new D3D12RHITexture();
}

void D3D12RHI::MakeSwapChainReady(RHICommandList* list)
{
	D3D12CommandList* dlist = DXConv(list);
	if (SwapChain->m_RenderTargetResources[m_frameIndex]->GetCurrentState() != D3D12_RESOURCE_STATE_PRESENT)
	{
		SwapChain->m_RenderTargetResources[m_frameIndex]->SetResourceState(dlist, D3D12_RESOURCE_STATE_PRESENT);
	}
}

RHIFence* D3D12RHI::CreateFence(DeviceContext* device, EFenceFlags::Type Flags)
{
	return new DXFence(device,Flags);
}

#if RHI_SUPPORTS_RT
RHIStateObject* D3D12RHI::CreateStateObject(DeviceContext* Device, RHIStateObjectDesc Desc)
{
#if WIN10_1809
	return new D3D12StateObject(Device, Desc);
#else
	return RHIClass::CreateStateObject(Device, Desc);
#endif
}

LowLevelAccelerationStructure* D3D12RHI::CreateLowLevelAccelerationStructure(DeviceContext * Device, const AccelerationStructureDesc & Desc)
{
#if WIN10_1809
	return new D3D12LowLevelAccelerationStructure(Device, Desc);
#else
	return RHIClass::CreateLowLevelAccelerationStructure(Device, Desc);
#endif
}

HighLevelAccelerationStructure* D3D12RHI::CreateHighLevelAccelerationStructure(DeviceContext * Device, const AccelerationStructureDesc & Desc)
{
#if WIN10_1809
	return new D3D12HighLevelAccelerationStructure(Device, Desc);
#else
	return RHIClass::CreateHighLevelAccelerationStructure(Device, Desc);
#endif
}
#endif
void D3D12RHI::DestroyContext()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	ReleaseUploadHeaps(true);
#if SUPPORT_DXGI
	SafeRelease(SwapChain);
//	SafeRelease(factory);
#endif
	MemoryUtils::DeleteCArray(DeviceContexts, MAX_GPU_DEVICE_COUNT);

	SafeDelete(ScreenShotter);
	ReportObjects();
}

void EnableShaderBasedValidation()
{
#if SUPPORT_DXGI
	ID3D12Debug* spDebugController0;
	ID3D12Debug1* spDebugController1;
	(D3D12GetDebugInterface(ID_PASS(&spDebugController0)));
	(spDebugController0->QueryInterface(ID_PASS(&spDebugController1)));
	spDebugController1->SetEnableGPUBasedValidation(true);
#endif
}


void D3D12RHI::ReportObjects()
{
#if SUPPORT_DXGI
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
#endif
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
#ifdef PLATFORM_WINDOWS
	IDXGraphicsAnalysis* pGraphicsAnalysis;
	HRESULT getAnalysis = DXGIGetDebugInterface1(0, __uuidof(pGraphicsAnalysis), reinterpret_cast<void**>(&pGraphicsAnalysis));
	if (getAnalysis != S_OK)
	{
		return false;
	}
	return true;
#else
	return false;
#endif
}

void D3D12RHI::WaitForGPU()
{
	WaitForAllGPUS();
}

void D3D12RHI::DisplayDeviceDebug()
{
	//Log::OutS << "Primary Adaptor Has " << GetMemory() << Log::OutS;1
}

RHIClass::GPUMemoryReport D3D12RHI::ReportMemory()
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
	GPUMemoryReport Report = {};
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (DeviceContexts[i] != nullptr)
		{
			Report.GpuData.push_back(DeviceContexts[i]->GetMemoryReport());
		}
	}
	return Report;
}

void D3D12RHI::LoadPipeLine()
{
#if 	SUPPORT_DXGI
#ifdef _DEBUG
#if !AFTERMATH
#define RUNDEBUG 1
#endif
#else 	
#define RUNDEBUG 0
#endif

	
#if RUNDEBUG //nsight needs this off
#if !_DEBUG
	Log::LogMessage("Validation layer running", Log::Warning);
#endif
	if (DetectGPUDebugger())
	{
		ForceNoDebug.SetValue(true);
	}
	if (!ForceNoDebug.GetBoolValue() && !DetectGPUDebugger())
	{	//EnableShaderBasedValidation();

// Enable the debug layer (requires the Graphics Tools "optional feature").
// NOTE: Enabling the debug layer after device creation will invalidate the active device.
		if (SUCCEEDED(D3D12GetDebugInterface(ID_PASS(&debugController))))
		{
			debugController->EnableDebugLayer();
			debugController->Release();
			// Enable additional debug layers.
			
		}
	}
#endif
	if (EnableDred.GetBoolValue() && !ForceNoDebug.GetBoolValue())
	{
#if WIN10_1903
		ID3D12DeviceRemovedExtendedDataSettings* pDredSettings;
		if (SUCCEEDED(D3D12GetDebugInterface(ID_PASS(&pDredSettings))))
		{
			pDredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			pDredSettings->SetWatsonDumpEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			pDredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		}
#endif
	}	
	ReportObjects();
#endif
	DX12DeviceInterface::CreateDevices(this,!ForceNoDebug.GetBoolValue());
	LinkGPUs();	
#if RUNDEBUG
	if (!ForceNoDebug.GetBoolValue())
	{
		ID3D12InfoQueue* infoqueue[MAX_GPU_DEVICE_COUNT] = { nullptr };
		for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
		{
			if (DeviceContexts[i] != nullptr)
			{
				DeviceContexts[i]->GetDevice()->QueryInterface(ID_PASS(&infoqueue[i]));
				if (infoqueue[i] != nullptr)
				{
					infoqueue[i]->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
					infoqueue[i]->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
					infoqueue[i]->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
					//	infoqueue[i]->AddMessage(D3D12_MESSAGE_CATEGORY::D3D12_MESSAGE_CATEGORY_INITIALIZATION, D3D12_MESSAGE_SEVERITY_WARNING, D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_LIVE_SAMPLER, "Init complete");
					infoqueue[i]->Release();
				}
			}
		}
	}
#endif
	DisplayDeviceDebug();
	//Log::LogMessage(ReportMemory());
}

void D3D12RHI::LinkGPUs()
{
	//#DX12: handle 3 GPU
	if (GetSecondaryDevice() != nullptr)
	{
		GetPrimaryDevice()->LinkAdaptors(GetSecondaryDevice());
		GetSecondaryDevice()->LinkAdaptors(GetPrimaryDevice());
	}
}

void D3D12RHI::HandleDeviceFailure()
{
	ensure(Instance);
	HRESULT HR;
	HR = Instance->GetPrimaryDevice()->GetDevice()->GetDeviceRemovedReason();
	ensureMsgf(HR == S_OK, (std::string)D3D12Helpers::DXErrorCodeToString(HR));
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
#if SUPPORT_OPENVR 
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
#endif
}

void D3D12RHI::ResizeSwapChain(int x, int y)
{
	if (x == 0 || y == 0)
	{
		return;
	}
	GetPrimaryDevice()->WaitForGpu();
#if SUPPORT_DXGI
	if (SwapChain != nullptr)
	{
		if (ScreenShotter != nullptr)
		{
			SafeDelete(ScreenShotter);
		}
		SwapChain->Resize(x, y);

		m_frameIndex = SwapChain->GetSwapChainIndex();
		ScreenShotter = new D3D12ReadBackCopyHelper(RHI::GetDefaultDevice(), SwapChain->m_RenderTargetResources[0]);
	}
#endif
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


void D3D12RHI::InitSwapChain()
{

	SwapChain = new SwapchainInferface();
#if SUPPORT_DXGI
	SwapChain->factory = DX12DeviceInterface::GetFactory();
#endif
	SwapChain->Init(RHI::CPUFrameCount, 0, glm::ivec2(m_width, m_height));
	ScreenShotter = new D3D12ReadBackCopyHelper(RHI::GetDefaultDevice(), SwapChain->m_RenderTargetResources[0], true);

}

void D3D12RHI::SetFullScreenState(bool state)
{
	IsFullScreen = state;
#if 	SUPPORT_DXGI
	//if (SwapChain->m_swapChain != nullptr)
	//{
	//	m_swapChain->SetFullscreenState(state, nullptr);
	//}
#endif
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


	//only set up to grab the 0 frame of spawn chain
	if (RunScreenShot && m_frameIndex == 0)
	{
		ScreenShotter->WriteBackRenderTarget();
		//#DX12: should be an async task to write from GPU to disk also prevent capture before write is complete
		ScreenShotter->WriteToFile(AssetManager::DirectGetGeneratedDir());
		Log::LogMessage("Took ScreenShot");
		RunScreenShot = false;
	}

	if (!RHI::AllowCPUAhead())
	{
		WaitForAllGPUS();
	}

	UpdateAllCopyEngines();
	SwapChain->Present();
	m_frameIndex = SwapChain->GetSwapChainIndex();
	
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (DeviceContexts[i] != nullptr)
		{
			DeviceContexts[i]->MoveNextFrame(m_frameIndex);
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
	SwapChain->ClearRenderTarget(MainList);
}

void D3D12RHI::RenderToScreen(ID3D12GraphicsCommandList* list)
{
	SwapChain->RenderToScreen(list);
}

void D3D12RHI::SetScreenRenderTarget(D3D12CommandList* list)
{
	SwapChain->SetScreenRenderTarget(list);
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

RHIBuffer * D3D12RHI::CreateRHIBuffer(ERHIBufferType::Type type, DeviceContext* Device)
{
	return new D3D12Buffer(type, Device);
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

bool D3D12RHI::InitRHI(bool initAllGPUs)
{
	InitAllGPUs = initAllGPUs;
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
	return new D3D12RHITextureArray(Device, Length);
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

