#include "RHI.h"
#include "Core/Assets/Archive.h"
#include "Core/Assets/Asset_Shader.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Assets/ImageIO.h"
#include "Core/Assets/ShaderComplier.h"
#include "Core/Input/Input.h"
#include "Core/Module/ModuleManager.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Platform/Windows/WindowsWindow.h"
#include "Core/Utils/VectorUtils.h"
#include "Rendering/Core/Defaults.h"
#include "Rendering/Core/DynamicQualityEngine.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/Material.h"
#include "Rendering/Core/ParticleSystemManager.h"
#include "Rendering/Performance/GPUPerformanceTestManager.h"
#include "Rendering/RayTracing/RayTracingEngine.h"
#include "Rendering/RenderNodes/RenderGraphSystem.h"
#include "Rendering/Utils/UAVFormatConverter.h"
#include "Rendering/VR/HMD.h"
#include "Rendering/VR/HMDManager.h"
#include "RHI_inc.h"
#include "RHIInterGPUStagingResource.h"
#include "RHIRenderPassCache.h"
#include "SFRController.h"
#include "Streaming/TextureStreamingEngine.h"
#include "Testing/EngineTests.h"

static ConsoleVariable RunTests("Test", 0, ECVarType::LaunchOnly);
static ConsoleVariable RunTestsExit("Testexit", 0, ECVarType::LaunchOnly);
RHI* RHI::instance = nullptr;
static ConsoleVariable StartFullscreen("fullscreen", 0, ECVarType::LaunchOnly);

RHI::RHI(ERenderSystemType system)
{
	GlobalDeleteQueue = new FrameCountingVector<IRHIResourse>();
	CurrentSystem = system;
	Rendersettings.ValidateSettings();
	Rendersettings.ValidateForAPI(CurrentSystem);
	RHIModule* RHImodule = nullptr;
	switch (CurrentSystem)
	{
#if BUILD_D3D12
	case ERenderSystemType::RenderSystemD3D12:
		Log::LogMessage("Loading DirectX 12 RHI");
		RHImodule = ModuleManager::Get()->GetModule<RHIModule>("D3D12RHI");
		ensure(RHImodule); 
		CurrentRHI = RHImodule->GetRHIClass();
		break;
#endif
#if BUILD_VULKAN
	case ERenderSystemType::RenderSystemVulkan:
		Log::LogMessage("Loading Vulkan RHI");
		RHImodule = ModuleManager::Get()->GetModule<RHIModule>("VulkanRHI");
		ensure(RHImodule);
		CurrentRHI = RHImodule->GetRHIClass();
		break;
#endif
	default:
		ensureFatalMsgf(false, "Selected RHI not Avalable");
		break;
	}
	ensureFatalMsgf(CurrentRHI, "RHI load failed");
}

RHI::~RHI()
{
	SafeDelete(HeadSetManager);
}

void RHI::InitRHI(ERenderSystemType e)
{
	if (instance == nullptr)
	{
		instance = new RHI(e);
	}
}

ERenderSystemType RHI::GetType()
{
	return instance->CurrentSystem;
}

RHIClass * RHI::GetRHIClass()
{
	if (instance != nullptr)
	{
		return instance->CurrentRHI;
	}
	return nullptr;
}

void RHI::WaitForGPU()
{
	GetRHIClass()->WaitForGPU();
}

const RenderConstants* RHI::GetRenderConstants()
{
	if (instance != nullptr)
	{
		return &instance->M_RenderConsants;
	}
	return nullptr;
}


RenderSettings* RHI::GetRenderSettings()
{
	if (instance != nullptr)
	{
		return &instance->Rendersettings;
	}
	return nullptr;
}

bool RHI::IsD3D12()
{
	return (GetType() == RenderSystemD3D12);
}

bool RHI::IsVulkan()
{
	return  (GetType() == RenderSystemVulkan);
}

bool RHI::SupportsThreading()
{
	return (GetType() == RenderSystemD3D12) || (GetType() == RenderSystemVulkan);
}

bool RHI::SupportsExplictMultiAdaptor()
{
	return (GetType() == RenderSystemD3D12) || (GetType() == RenderSystemVulkan);
}

bool RHI::AllowIHVAcceleration()
{
	return true;
}

bool RHI::SupportsUnlinkedAdaptors()
{
	//vulkan only supports GPUs linked in drivers :(
	return (GetType() == RenderSystemD3D12);
}

RHIBuffer * RHI::CreateRHIBuffer(ERHIBufferType::Type type, DeviceContext* Device)
{
	return GetRHIClass()->CreateRHIBuffer(type, Device);
}

RHICommandList * RHI::CreateCommandList(ECommandListType::Type Type, DeviceContext * Device)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}
	return GetRHIClass()->CreateCommandList(Type, Device);
}

RHIGPUSyncEvent * RHI::CreateSyncEvent(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue, DeviceContext * Device, DeviceContext * SignalDevice)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}
	return GetRHIClass()->CreateSyncEvent(WaitingQueue, SignalQueue, Device, SignalDevice);
}

bool RHI::BlockCommandlistExec()
{
	return false;
}

bool RHI::AllowCPUAhead()
{
	return true;
}

int RHI::GetFrameCount()
{
	if (instance == nullptr)
	{
		return 0;
	}
	return instance->PresentCount;
}

int RHI::GetDeviceCount()
{
	for (int i = MAX_GPU_DEVICE_COUNT - 1; i >= 0; i--)
	{
		if (GetDeviceContext(i) != nullptr)
		{
			return i + 1;
		}
	}
	return 1;
}

bool RHI::SupportVR()
{
#if RHI_SUPPORTS_VR
	return instance->Rendersettings.VRHMDMode != EVRHMDMode::Disabled;
#else
	return false;
#endif
}

void RHI::DetectAndInitVR()
{
#if RHI_SUPPORTS_VR
	HeadSetManager = new HMDManager();
	HeadSetManager->Init();
#endif
}

HMD * RHI::GetHMD()
{
#if RHI_SUPPORTS_VR
	return instance->HeadSetManager->GetHMD();
#else
	return nullptr;
#endif
}

HMDManager * RHI::GetHMDManager()
{
#if RHI_SUPPORTS_VR
	return instance->HeadSetManager;
#else 
	return nullptr;
#endif
}

bool RHI::IsRenderingVR()
{
	return SupportVR() && GetHMD() != nullptr && GetHMD()->IsActive();
}

bool RHI::UseAdditionalGPUs()
{
	return true;
}

VRSettings * RHI::GetVrSettings()
{
	return &instance->HeadSetManager->VrSettings;
}

RHI * RHI::Get()
{
	return instance;
}

SFRController * RHI::GetSplitController()
{
	return instance->SFR_Controller;
}

void RHI::FlushDeferredDeleteQueue()
{
	Get()->TickDeferredDeleteQueue(true);
}

void RHI::Tick()
{
	Get()->QualityEngine->Update();
	instance->SFR_Controller->Tick();
	TextureStreamingEngine::Get()->Update();
}

void RHI::SubmitToVRComposter(FrameBuffer * fb, EEye::Type eye)
{
	GetRHIClass()->SubmitToVRComposter(fb, eye);
}

RHIRenderPassDesc RHI::GetRenderPassDescForSwapChain(bool ClearScreen /*= false*/)
{
	return GetRHIClass()->GetRenderPassDescForSwapChain(ClearScreen);
}

RenderGraphSystem * RHI::GetRenderSystem()
{
	return Get()->RenderSystem;
}

GPUPerformanceTestManager * RHI::GetTestManager()
{
	return Get()->TestManager;
}

void RHI::RunGPUTests()
{
	Get()->TestManager->ExecuteAllTests();
}

void RHI::MakeSwapChainReady(RHICommandList * list)
{
	GetRHIClass()->MakeSwapChainReady(list);
	Get()->CheckSwapReady = true;
}

void RHI::AddToDeferredDeleteQueue(IRHIResourse * Resource)
{
	LogEnsure(!Resource->IsPendingKill());
	ensure(Resource->GetRefCount() == 0);
	if (Resource->IsPendingKill())
	{
		return;
	}
	if (VectorUtils::Contains(Get()->DeferredDeleteQueue, RHIResourseStamped(Resource, RHI::GetFrameCount())))
	{
		DEBUGBREAK;
	}
	if (Get()->IsFlushingDeleteQueue)
	{
		SafeRHIRelease(Resource);
	}
	else
	{
		//Get()->DeferredDeleteQueue.push_back(RHIResourseStamped(Resource, RHI::GetFrameCount()));
		Get()->GlobalDeleteQueue->Enqeueue(Resource);
		Resource->PendingKill = true;
	}
}
static ConsoleVariable IsWithNsight("Nsight", 0, ECVarType::LaunchOnly);
void RHI::TickDeferredDeleteQueue(bool Flush /*= false*/)
{
	//#DX12 Nsight crashes here for some reason
	if (IsWithNsight.GetBoolValue())
	{
		return;
	}
	SCOPE_CYCLE_COUNTER_GROUP("TickDeferredDeleteQueue", "RHI");
	IsFlushingDeleteQueue = true;
	GlobalDeleteQueue->Tick([](IRHIResourse* R) { SafeRHIRelease(R); return true; }, Flush);
	IsFlushingDeleteQueue = false;
}

void RHI::DestoryRHI()
{
	if (instance != nullptr)
	{
		SafeDelete(instance);
	}
}

#define NOLOADTEX 0
BaseTextureRef RHI::CreateTexture(AssetPathRef path, DeviceContext* Device, RHITextureDesc Desc)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}
	//#Textures: Default Cube Map!
#if NOLOADTEX	
	if (ImageIO::GetDefaultTexture() && !Desc.IsCubeMap)
	{
		return ImageIO::GetDefaultTexture();
	}
#endif
	BaseTextureRef newtex = nullptr;
	if (ImageIO::CheckIfLoaded(path.GetRelativePathToAsset(), &newtex))
	{
		return newtex;
	}
	newtex = GetRHIClass()->CreateTexture(Desc, Device);
	if (!newtex->CreateFromFile(path))
	{
		return ImageIO::GetDefaultTexture();
	}
	if (/*Desc.InitOnALLDevices && */Device->GetDeviceIndex() == 0 && RHI::GetDeviceCount() > 1 && RHI::IsD3D12() /*&& RHI::GetRenderSettings()->InitSceneDataOnAllGPUs*/)
	{
		BaseTextureRef other = GetRHIClass()->CreateTexture(Desc, RHI::GetDeviceContext(1));
		newtex->RegisterOtherDeviceTexture(other.Get());
		other->CreateFromFile(path);
	}
	ImageIO::RegisterTextureLoad(newtex);
	return newtex;
}

BaseTexture * RHI::CreateTextureWithData(int with, int height, int nChannels, void * data, DeviceContext* Device)
{
	return GetRHIClass()->CreateTexture(RHITextureDesc(), Device);
}

BaseTexture * RHI::CreateNullTexture(DeviceContext * Device)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}
	BaseTexture* newtex = nullptr;
	newtex = GetRHIClass()->CreateTexture(RHITextureDesc(), Device);
	newtex->CreateAsNull();
	return newtex;
}

Mesh * RHI::CreateMesh(const char * path)
{
	MeshLoader::FMeshLoadingSettings set = MeshLoader::FMeshLoadingSettings();
	return CreateMesh(path, set);
}

Mesh * RHI::CreateMesh(const char * path, MeshLoader::FMeshLoadingSettings& Settings)
{
	if (Settings.AllowInstancing)
	{
		Mesh* New = MeshLoader::Get()->TryLoadFromCache(path);
		if (New != nullptr && New->GetSkeletalMesh() == nullptr)
		{
			return New;
		}
	}
	std::string accpath = AssetManager::GetContentPath();
	accpath.append(path);
	AssetManager::RegisterMeshAssetLoad(path);
	Mesh* newmesh = new Mesh(accpath, Settings);
	newmesh->AssetName = path;
	if (Settings.AllowInstancing)
	{
		MeshLoader::RegisterLoad(path, newmesh);
	}
	return newmesh;
}

FrameBuffer* RHI::CreateFrameBuffer(DeviceContext* Device, const RHIFrameBufferDesc& Desc, bool AutoSizeDesc/* = false*/)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}
	if (AutoSizeDesc)
	{
		RHIFrameBufferDesc NewDesc = Desc;
		FrameBuffer::AutoUpdateSize(NewDesc);
		return GetRHIClass()->CreateFrameBuffer(Device, NewDesc);
	}
	return GetRHIClass()->CreateFrameBuffer(Device, Desc);
}

DeviceContext* RHI::GetDeviceContext(int index)
{
	return GetRHIClass()->GetDeviceContext(index);
}

ShaderProgramBase * RHI::CreateShaderProgam(DeviceContext* Device)
{
	return GetRHIClass()->CreateShaderProgam(Device);
}

DeviceContext * RHI::GetDefaultDevice()
{
	return GetRHIClass()->GetDefaultDevice();
}

RHIPipeLineStateObject * RHI::CreatePipelineStateObject(const RHIPipeLineStateDesc & Desc, DeviceContext * Device)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}
	RHIPipeLineStateObject* PSO = GetRHIClass()->CreatePSO(Desc, Device);
	PSO->Complie();
	Device->GetPSOCache()->AddToCache(PSO);
	return PSO;
}

RHIQuery * RHI::CreateQuery(EGPUQueryType::Type type, DeviceContext * con)
{
	ValidateDevice(con);
	return GetRHIClass()->CreateQuery(type, con);
}

void RHI::InitialiseContext()
{
	ShaderComplier::Get();//create the Complier
	instance->RenderPassCache = new RHIRenderPassCache();
	new TextureStreamingEngine();
	GetRHIClass()->InitRHI(instance->Rendersettings.RequestAllGPUs);
	PlatformWindow::TickSplashWindow(5, "Loading RHI");
	instance->ValidateSettings();
	RunTests.SetValue(true);
#if RUNTESTS
	if (RunTests.GetBoolValue())
	{
		TESTING::RunTests();
		RHI::FlushDeferredDeleteQueue();
		if (RunTestsExit.GetBoolValue())
		{
			Engine::ImmediateExit(0);
			return;
		}
	}
#endif
	ShaderComplier::Get()->Init();
	ShaderComplier::Get()->ComplieAllGlobalShaders();
	PlatformWindow::TickSplashWindow(10, "Loading Render Graph");
	ParticleSystemManager::Get();
	instance->SFR_Controller = new SFRController();
	Defaults::Start();
	Input::Startup();
	instance->DetectAndInitVR();
	if (RHI::GetRenderSettings()->GetRTSettings().Enabled)
	{
		instance->RTE = new RayTracingEngine();
	}
	Get()->QualityEngine = new DynamicQualityEngine();
	Get()->RenderSystem = new RenderGraphSystem();
	Get()->TestManager = new GPUPerformanceTestManager();
	Get()->TestManager->Init();
	UAVFormatConverter::Get();
	
}

void RHI::ValidateSettings()
{

}

void RHI::InitialiseContextWindow(int w, int h)
{
	GetRHIClass()->InitWindow(w, h);
	instance->SwapChainWidth = w;
	instance->SwapChainHeight = h;

}

std::string RHI::ReportMemory(bool all /*= false*/)
{
	std::string MemoryReport = "";
	RHIClass::GPUMemoryReport report = GetRHIClass()->ReportMemory();
	for (int i = 0; i < report.GpuData.size(); i++)
	{
		MemoryReport += "GPU" + std::to_string(i) + ": " + StringUtils::ByteToMB(report.GpuData[i].TotalAllocated) + " / " + StringUtils::ByteToMB(report.GpuData[i].MaxBudget);
		if (all)
		{
			MemoryReport += "(Tracking Delta " + StringUtils::ByteToMB(report.GpuData[i].UntrackedDelta) + ")";
		}
	}
	return MemoryReport;
}

void RHI::RHISwapBuffers()
{
	ensure(Get()->CheckSwapReady);
	GetRHIClass()->RHISwapBuffers();
	Get()->TickDeferredDeleteQueue();
	instance->PresentCount++;
	Get()->CheckSwapReady = false;
}

void RHI::RHIRunFirstFrame()
{
	if (RHI::GetFrameCount() != 0)
	{
		return;
	}
	GetRHIClass()->RHIRunFirstFrame();
	ShaderComplier::Get()->TickMaterialComplie();
	GetRHIClass()->SetFullScreenState(StartFullscreen.GetBoolValue());
	WaitForGPU();
}

void RHI::SetFullScreenState(bool state)
{
	Get()->IsFullScreen = state;
	GetRHIClass()->SetFullScreenState(state);
}

void RHI::ToggleFullScreenState()
{
	SetFullScreenState(!Get()->IsFullScreen);
}

void RHI::AddLinkedFrameBuffer(FrameBuffer* target, bool NoResize /*= false*/)
{
	if (instance != nullptr)
	{
		ensure(target != nullptr);
		instance->FrameBuffersLinkedToSwapChain.push_back(target);
		if (!NoResize)
		{
			ResizeFrameBuffer(target);
		}
	}
}

void RHI::ValidateDevice(DeviceContext *& con)
{
	if (con == nullptr)
	{
		con = RHI::GetDefaultDevice();
	}
}

void RHI::ResizeFrameBuffer(FrameBuffer* target)
{
	int Width = 0;
	int Height = 0;
	if (RHI::GetRenderSettings()->LockBackBuffer)
	{
		Width = (int)(target->GetDescription().LinkToBackBufferScaleFactor* RHI::GetRenderSettings()->LockedWidth);
		Height = (int)(target->GetDescription().LinkToBackBufferScaleFactor* RHI::GetRenderSettings()->LockedHeight);
	}
	else
	{
		Width = (int)(target->GetDescription().LinkToBackBufferScaleFactor* instance->SwapChainWidth);
		Height = (int)(target->GetDescription().LinkToBackBufferScaleFactor* instance->SwapChainHeight);
	}
	target->Resize(Width, Height);
	if (target->GetDescription().LinkToBackBufferScaleFactor != 1.0f)
	{
		Log::LogMessage("SwapChain Linked Buffer was created at " + std::to_string(Width) + "X" + std::to_string(Height));
	}

}
void RHI::RemoveLinkedFrameBuffer(FrameBuffer * target)
{
	VectorUtils::Remove(instance->FrameBuffersLinkedToSwapChain, target);
}

void RHI::ResizeSwapChain(int width, int height)
{
	instance->SwapChainWidth = width;
	instance->SwapChainHeight = height;
	GetRHIClass()->ResizeSwapChain(width, height);
	for (int i = 0; i < instance->FrameBuffersLinkedToSwapChain.size(); i++)
	{
		ResizeFrameBuffer(instance->FrameBuffersLinkedToSwapChain[i]);
	}
}

void RHI::DestoryContext()
{
	if (Get())
	{
		SafeDelete(Get()->RenderSystem);
	}
	Defaults::Shutdown();
	if (GetRHIClass())
	{
		GetRHIClass()->WaitForGPU();
		ParticleSystemManager::ShutDown();
	}
	TextureStreamingEngine::ShutDown();
	ShaderComplier::Get()->FreeAllGlobalShaders();
	if (Get())
	{
		Get()->TickDeferredDeleteQueue(true);
		GetRHIClass()->DestoryRHI();
	}
#if DETECT_MEMORY_LEAKS
	RefCheckerContainer::LogAllRefCounters();
#endif
}

void RHI::BeginFrame()
{
	for (int i = 0; i < GetDeviceCount(); i++)
	{
		RHI::GetDeviceContext(i)->OnFrameStart();
	}
}

RHITextureArray * RHI::CreateTextureArray(DeviceContext* Device, int Length)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}
	return GetRHIClass()->CreateTextureArray(Device, Length);
}

PipelineStateObjectCache::PipelineStateObjectCache(DeviceContext * dev)
{
	Device = dev;
}

PipelineStateObjectCache::~PipelineStateObjectCache()
{
	Destory();
}

RHIPipeLineStateObject* PipelineStateObjectCache::GetFromCache(RHIPipeLineStateDesc& desc)
{
	desc.Build();
#if PSO_USE_MAP
	auto itor = PSOMap.find(desc.GetHash());

	if (itor == PSOMap.end())
	{
		return RHI::CreatePipelineStateObject(desc, Device);
	}
	ensure(itor->second->GetDesc() == desc);
	return itor->second;
#else
	std::lock_guard<std::mutex> lock(CacheLock);
	for (int i = 0; i < PSOMap.size(); i++)
	{
		if (PSOMap[i]->GetDesc() == desc)
		{
			return PSOMap[i];
	}
}
	return RHI::CreatePipelineStateObject(desc, Device);
#endif
}

void PipelineStateObjectCache::AddToCache(RHIPipeLineStateObject * object)
{
#if PSO_USE_MAP
	PSOMap.emplace(object->GetDescHash(), object);
#else
	PSOMap.push_back(object);
#endif
}

void PipelineStateObjectCache::Destory()
{
#if PSO_USE_MAP
	for (auto itor = PSOMap.begin(); itor != PSOMap.end(); itor++)
	{
		SafeRelease(itor->second);
}
#else
	MemoryUtils::DeleteReleaseableVector(PSOMap);
#endif
	PSOMap.clear();
}

void RHIClass::SubmitToVRComposter(FrameBuffer * fb, EEye::Type eye)
{}

LowLevelAccelerationStructure * RHIClass::CreateLowLevelAccelerationStructure(DeviceContext * Device, const AccelerationStructureDesc & Desc)
{
	return nullptr;
}

HighLevelAccelerationStructure* RHIClass::CreateHighLevelAccelerationStructure(DeviceContext * Device, const AccelerationStructureDesc & Desc)
{
	return nullptr;
}

RHIStateObject* RHIClass::CreateStateObject(DeviceContext* Device, RHIStateObjectDesc Desc)
{
	return nullptr;
}

RHIRenderPassDesc RHIClass::GetRenderPassDescForSwapChain(bool ClearScreen /*= false*/)
{
	RHIRenderPassDesc Desc;
	Desc.TargetSwapChain = true;
	Desc.LoadOp = ClearScreen ? ERenderPassLoadOp::Clear : ERenderPassLoadOp::Load;
	if (!ClearScreen)
	{
		Desc.InitalState = EResourceState::RenderTarget;
	}
	return  Desc;
}
