#include "RHI.h"
#include "BaseTexture.h"
#include "Rendering/Core/Renderable.h"
#include "Rendering/Core/Mesh.h"
#include "RHI/ShaderBase.h"
#include "ShaderProgramBase.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Assets/AssetManager.h"
#include "Rendering/Core/Mesh.h"
#include "Rendering/Core/GPUStateCache.h"
#include "Core/Engine.h"
#include "RHICommandList.h"
#include "Core/Assets/ImageIO.h"

#if BUILD_D3D12
#include "RHI/RenderAPIs/D3D12/D3D12Texture.h"
#include "RHI/RenderAPIs/D3D12/D3D12Shader.h"
#include "RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include "RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#endif

#if BUILD_VULKAN
#include "RHI/RenderAPIs/Vulkan/VKanRHI.h"
#include "RHI/RenderAPIs/Vulkan/VKanCommandlist.h"
#include "RHI/RenderAPIs/Vulkan/VKanFramebuffer.h"
#include "RHI/RenderAPIs/Vulkan/VKanShader.h"
#include "RHI/RenderAPIs/Vulkan/VKanTexture.h"
#endif

RHI* RHI::instance = nullptr;
MultiGPUMode RHI::CurrentMGPUMode = MultiGPUMode();
RHI::RHI(ERenderSystemType system)
{
	CurrentSystem = system;
	switch (CurrentSystem)
	{
#if BUILD_D3D12
	case ERenderSystemType::RenderSystemD3D12:
			CurrentRHI = new D3D12RHI();
			break;
#endif
#if BUILD_VULKAN
	case ERenderSystemType::RenderSystemVulkan:
		CurrentRHI = new VKanRHI();
		break;
#endif
	default:
		ensureMsgf(false, "Selected RHI not Avalable");
		break;
	}
	
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

RHIBuffer * RHI::CreateRHIBuffer(RHIBuffer::BufferType type, DeviceContext* Device)
{
	return	GetRHIClass()->CreateRHIBuffer(type, Device);
}

RHIUAV * RHI::CreateUAV(DeviceContext * Device)
{
	
	return GetRHIClass()->CreateUAV(Device);;
}

RHICommandList * RHI::CreateCommandList(ECommandListType::Type Type, DeviceContext * Device)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}	
	return GetRHIClass()->CreateCommandList(Type, Device);;
}

bool RHI::BlockCommandlistExec()
{
	return false;
}

bool RHI::AllowCPUAhead()
{
	return true;
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

bool RHI::UseAdditonalGPUs()
{
	return false;
}

const MultiGPUMode * RHI::GetMGPUMode()
{
	return &CurrentMGPUMode;
}

void RHI::DestoryRHI()
{
	if (instance != nullptr)
	{
		delete instance;
	}
}

#define NOLOADTEX 0
BaseTexture * RHI::CreateTexture(AssetManager::AssetPathRef path, DeviceContext* Device)
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

Renderable * RHI::CreateMesh(const char * path)
{
	return CreateMesh(path, MeshLoader::FMeshLoadingSettings());
}

Renderable * RHI::CreateMesh(const char * path, MeshLoader::FMeshLoadingSettings& Settings)
{
	///todo asset paths
	std::string accpath = Engine::GetRootDir();
	std::string apath = ("\\asset\\models\\");
	apath.append(path);
	accpath.append(apath);
	AssetManager::RegisterMeshAssetLoad(apath);
	return new Mesh(accpath, Settings);
}

FrameBuffer * RHI::CreateFrameBuffer(DeviceContext * Device, RHIFrameBufferDesc & Desc)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}
	return GetRHIClass()->CreateFrameBuffer(Device, Desc);
}


DeviceContext* RHI::GetDeviceContext(int index)
{
	if (IsD3D12())
	{
		return GetRHIClass()->GetDeviceContext(index);
	}
	return nullptr;
}

ShaderProgramBase * RHI::CreateShaderProgam(DeviceContext* Device)
{
	return GetRHIClass()->CreateShaderProgam(Device);
}

DeviceContext * RHI::GetDefaultDevice()
{
	return GetRHIClass()->GetDefaultDevice();
}

bool RHI::InitialiseContext(int w, int h)
{	
	return GetRHIClass()->InitRHI(w, h);
}

void RHI::RHISwapBuffers()
{
	GetRHIClass()->RHISwapBuffers();
}

void RHI::RHIRunFirstFrame()
{
	GetRHIClass()->RHIRunFirstFrame();
}

void RHI::ToggleFullScreenState()
{
	GetRHIClass()->ToggleFullScreenState();
}

void RHI::ResizeSwapChain(int width, int height)
{
	GetRHIClass()->ResizeSwapChain(width, height);
}

void RHI::DestoryContext()
{
	GetRHIClass()->DestoryRHI();
}

RHITextureArray * RHI::CreateTextureArray(DeviceContext* Device, int Length)
{
	if (Device == nullptr)
	{
		Device = RHI::GetDefaultDevice();
	}	
	return GetRHIClass()->CreateTextureArray(Device, Length);;
}