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
		RHImodule = ModuleManager::Get()->GetModule<RHIModule>("D3D12RHI");
		ensure(RHImodule);
		CurrentRHI = RHImodule->GetRHIClass();
		break;
#endif
#if BUILD_VULKAN
	case ERenderSystemType::RenderSystemVulkan:
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
{}

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

void RHI::AddLinkedFrameBuffer(FrameBuffer * target)
{
	if (instance != nullptr)
	{
		ensure(target != nullptr);
		instance->FrameBuffersLinkedToSwapChain.push_back(target);
	}
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

RHIGPUSyncEvent * RHI::CreateSyncEvent(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue, DeviceContext * Device)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}
	return GetRHIClass()->CreateSyncEvent(WaitingQueue, SignalQueue, Device);
}

bool RHI::BlockCommandlistExec()
{
	return false;
}

bool RHI::AllowCPUAhead()
{
	return false;
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
	if (GetDeviceContext(1) != nullptr)
	{
		return 2;
	}
	if (GetDeviceContext(2) != nullptr)
	{
		return 3;
	}
	return 1;
}

bool RHI::UseAdditionalGPUs()
{
	return true;
}

const MultiGPUMode * RHI::GetMGPUMode()
{
	return &instance->CurrentMGPUMode;
}

RHI * RHI::Get()
{
	return instance;
}

SFRController * RHI::GetSplitController()
{
	return instance->SFR_Controller;
}

void RHI::AddToDeferredDeleteQueue(IRHIResourse * Resource)
{
	//ensure(!Resource->IsPendingKill());
	if (Resource->IsPendingKill())
	{
		return;
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
BaseTexture * RHI::CreateTexture(AssetPathRef path, DeviceContext* Device, RHITextureDesc Desc)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}
#if NOLOADTEX
	//TODO: Default Cube Map!
	if (ImageIO::GetDefaultTexture())
	{
		return ImageIO::GetDefaultTexture();
	}
#endif
	BaseTexture* newtex = nullptr;
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
		BaseTexture* other = GetRHIClass()->CreateTexture(Desc, RHI::GetDeviceContext(1));
		newtex->RegisterOtherDeviceTexture(other);
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
}

void RHI::ValidateSettings()
{
	CurrentMGPUMode.ValidateSettings();
	if (CurrentMGPUMode.SplitShadowWork)
	{
		ensureMsgf(RenderSettings.IsDeferred, "Multigpu shadows only supported on Deferred renderer");
	}
	if (CurrentMGPUMode.MainPassSFR)
	{
		ensureMsgf(!RenderSettings.IsDeferred, "SFR only supported in forward rendering mode");
	}
}

void RHI::InitialiseContextWindow(int w, int h)
{
	GetRHIClass()->InitWindow(w, h);
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

void RHI::ResizeSwapChain(int width, int height)
{
	GetRHIClass()->ResizeSwapChain(width, height);
	for (int i = 0; i < instance->FrameBuffersLinkedToSwapChain.size(); i++)
	{
		instance->FrameBuffersLinkedToSwapChain[i]->Resize(width, height);
	}
}

void RHI::DestoryContext()
{
	if (GetRHIClass())
	{
		GetRHIClass()->WaitForGPU();
		ParticleSystemManager::Get()->ShutDown();
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

