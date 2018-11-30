#include "RHI.h"
#include "Core/Assets/AssetManager.h"
#include "RHI_inc.h"
#include "Core/Assets/ImageIO.h"
#include "Core/Module/ModuleManager.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Assets/ShaderComplier.h"
#include "Rendering/Core/ParticleSystemManager.h"
#include "Core/Platform/ConsoleVariable.h"
RHI* RHI::instance = nullptr;
MultiGPUMode RHI::CurrentMGPUMode = MultiGPUMode();
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
	return &CurrentMGPUMode;
}



RHI * RHI::Get()
{
	return instance;
}

void RHI::AddToDeferredDeleteQueue(IRHIResourse * Resource)
{
	ensure(!Resource->IsPendingKill());
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
BaseTexture * RHI::CreateTexture(AssetPathRef path, DeviceContext* Device)
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
	newtex = GetRHIClass()->CreateTexture(Device);
	if (!newtex->CreateFromFile(path))
	{
		return ImageIO::GetDefaultTexture();
	}
	ImageIO::RegisterTextureLoad(newtex);
	return newtex;
}

BaseTexture * RHI::CreateTextureWithData(int with, int height, int nChannels, void * data, DeviceContext* Device)
{
	return GetRHIClass()->CreateTexture(Device);
}

BaseTexture * RHI::CreateNullTexture(DeviceContext * Device)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}
	BaseTexture* newtex = nullptr;
	newtex = GetRHIClass()->CreateTexture(Device);
	newtex->CreateAsNull();
	return newtex;
}

Mesh * RHI::CreateMesh(const char * path)
{
	return CreateMesh(path, MeshLoader::FMeshLoadingSettings());
}

Mesh * RHI::CreateMesh(const char * path, MeshLoader::FMeshLoadingSettings& Settings)
{
	///todo asset paths
	std::string accpath = AssetManager::GetContentPath();
	std::string apath = "";/// ("\\models\\");
	apath.append(path);
	accpath.append(apath);//todo remove
	AssetManager::RegisterMeshAssetLoad(apath);
	Mesh* newmesh = new Mesh(accpath, Settings);
	newmesh->AssetName = path;
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

void RHI::InitialiseContext()
{	
	GetRHIClass()->InitRHI();
	CurrentMGPUMode.ValidateSettings();
	ShaderComplier::Get()->ComplieAllGlobalShaders();
	ParticleSystemManager::Get();
}

void RHI::InitialiseContextWindow(int w, int h)
{
	GetRHIClass()->InitWindow(w, h);
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
	return GetRHIClass()->CreateTextureArray(Device, Length);;
}