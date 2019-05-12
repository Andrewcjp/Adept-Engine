#include "RHI.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Assets/ImageIO.h"
#include "Core/Assets/ShaderComplier.h"
#include "Core/Module/ModuleManager.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Platform/PlatformCore.h"
#include "Rendering/Core/ParticleSystemManager.h"
#include "RHI_inc.h"
#include "Core/Utils/RefChecker.h"
#include "SFRController.h"
#include "Core/Assets/Archive.h"
#include "Rendering/Core/Material.h"
#include "Core/Assets/Asset_Shader.h"
#include "Rendering/Core/Defaults.h"
#include "Core/Utils/VectorUtils.h"
#include "Rendering/VR/HMD.h"
#include "Rendering/VR/HMDManager.h"

RHI* RHI::instance = nullptr;
static ConsoleVariable StartFullscreen("fullscreen", 0, ECVarType::LaunchOnly);

RHI::RHI(ERenderSystemType system)
{

	CurrentSystem = system;
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
		return &instance->RenderSettings;
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
	return (GetType() == RenderSystemD3D12);
}

RHIBuffer * RHI::CreateRHIBuffer(ERHIBufferType::Type type, DeviceContext* Device)
{
	return	GetRHIClass()->CreateRHIBuffer(type, Device);
}

RHIUAV * RHI::CreateUAV(DeviceContext * Device)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}
	return GetRHIClass()->CreateUAV(Device);
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
	return instance->RenderSettings.EnableVR;
}

void RHI::DetectAndInitVR()
{
	HeadSetManager = new HMDManager();
	HeadSetManager->Init();
}

HMD * RHI::GetHMD()
{
	return instance->HeadSetManager->GetHMD();
}

HMDManager * RHI::GetHMDManager()
{
	return instance->HeadSetManager;
}

bool RHI::UseAdditionalGPUs()
{
	return true;
}

const MultiGPUMode * RHI::GetMGPUSettings()
{
	return &instance->CurrentMGPUMode;
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
	instance->SFR_Controller->Tick();
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
		__debugbreak();
	}
	if (Get()->IsFlushingDeleteQueue)
	{
		SafeRHIRelease(Resource);
	}
	else
	{
		Get()->DeferredDeleteQueue.push_back(RHIResourseStamped(Resource, RHI::GetFrameCount()));
		Resource->PendingKill = true;
	}
}

void RHI::TickDeferredDeleteQueue(bool Flush /*= false*/)
{
	//#DX12 Nsight crashes here for some reason
	//return;
	IsFlushingDeleteQueue = true;
	for (int i = (int)DeferredDeleteQueue.size() - 1; i >= 0; i--)
	{
		const int CurrentFrame = RHI::GetFrameCount();
		if (DeferredDeleteQueue[i].second + RHI::CPUFrameCount < CurrentFrame || Flush)
		{
			SafeRHIRelease(DeferredDeleteQueue[i].first);
			DeferredDeleteQueue.erase(DeferredDeleteQueue.begin() + i);
		}
	}
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
	if (Desc.InitOnALLDevices && Device->GetDeviceIndex() == 0)
	{
		BaseTextureRef other = GetRHIClass()->CreateTexture(Desc, RHI::GetDeviceContext(1));
		newtex->RegisterOtherDeviceTexture(other.Get());
		other->CreateFromFile(path);
	}
	ImageIO::RegisterTextureLoad(newtex.Get());
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
	return CreateMesh(path, MeshLoader::FMeshLoadingSettings());
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

FrameBuffer* RHI::CreateFrameBuffer(DeviceContext* Device, const RHIFrameBufferDesc& Desc)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
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

void RHI::InitialiseContext()
{
	GetRHIClass()->InitRHI();
	instance->ValidateSettings();
	ShaderComplier::Get()->ComplieAllGlobalShaders();
	ParticleSystemManager::Get();
	instance->SFR_Controller = new SFRController();
	Defaults::Start();
	instance->DetectAndInitVR();
}

void RHI::ValidateSettings()
{
	CurrentMGPUMode.ValidateSettings();
}

void RHI::InitialiseContextWindow(int w, int h)
{
	GetRHIClass()->InitWindow(w, h);
	instance->SwapChainWidth = w;
	instance->SwapChainHeight = h;
}

std::string RHI::ReportMemory()
{
	return GetRHIClass()->ReportMemory();
}

void RHI::RHISwapBuffers()
{
	GetRHIClass()->RHISwapBuffers();
	Get()->TickDeferredDeleteQueue();
	instance->PresentCount++;
}

void RHI::RHIRunFirstFrame()
{
	GetRHIClass()->RHIRunFirstFrame();
	ShaderComplier::Get()->TickMaterialComplie();
	GetRHIClass()->SetFullScreenState(StartFullscreen.GetBoolValue());
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
	Defaults::Shutdown();
	if (GetRHIClass())
	{
		GetRHIClass()->WaitForGPU();
		ParticleSystemManager::ShutDown();
	}
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
#if PSO_USE_FULL_STRING_MAPS
	auto itor = PSOMap.find(desc.GetString());
#else
	auto itor = PSOMap.find(desc.GetHash());
#endif

	if (itor == PSOMap.end())
	{
		return RHI::CreatePipelineStateObject(desc, Device);
	}
	ensure(itor->second->GetDesc() == desc);
	return itor->second;
#else
	for (auto itor = PSOMap.begin(); itor != PSOMap.end(); itor++)
	{
		if (itor->second->GetDesc() == desc)
		{
			return itor->second;
		}
	}
	return RHI::CreatePipelineStateObject(desc, Device);
#endif
}

void PipelineStateObjectCache::AddToCache(RHIPipeLineStateObject * object)
{
#if PSO_USE_FULL_STRING_MAPS
	PSOMap.emplace(object->GetDescString(), object);
#else
	PSOMap.emplace(object->GetDescHash(), object);
#endif
}

void PipelineStateObjectCache::Destory()
{
	for (auto itor = PSOMap.begin(); itor != PSOMap.end(); itor++)
	{
		itor->second->Release();
	}
	PSOMap.clear();
}

