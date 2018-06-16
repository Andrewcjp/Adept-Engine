#include "RHI.h"
#include "BaseTexture.h"
#include "../Rendering/Core/Renderable.h"
#include "../Rendering/Core/Mesh.h"
#include "RHI/ShaderBase.h"
#include "ShaderProgramBase.h"
#include "../Core/Performance/PerfManager.h"
#include "../Core/Assets/AssetManager.h"
#include "../Rendering/Core/Mesh.h"
#include "../Rendering/Core/GPUStateCache.h"
#include "../Core/Engine.h"
#include "RHICommandList.h"
#include "../Core/Assets/ImageIO.h"

#if BUILD_D3D12
#include "../RHI/RenderAPIs/D3D12/D3D12Texture.h"
#include "../RHI/RenderAPIs/D3D12/D3D12Shader.h"
#include "../RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "../RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include "../RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#endif

#if BUILD_VULKAN
#include "../RHI/RenderAPIs/Vulkan/VKanRHI.h"
#include "../RHI/RenderAPIs/Vulkan/VKanCommandlist.h"
#include "../RHI/RenderAPIs/Vulkan/VKanFramebuffer.h"
#include "../RHI/RenderAPIs/Vulkan/VKanShader.h"
#include "../RHI/RenderAPIs/Vulkan/VKanTexture.h"
#endif

RHI* RHI::instance = nullptr;
RHI::RHI()
{}

RHI::~RHI()
{}

void RHI::InitRHI(ERenderSystemType e)
{
	if (instance == nullptr)
	{
		instance = new RHI();
	}
	instance->CurrentSystem = e;
}

ERenderSystemType RHI::GetType()
{
	return instance->CurrentSystem;
}

bool RHI::IsOpenGL()
{
	return (GetType() == RenderSystemOGL);
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
	switch (instance->CurrentSystem)
	{
#if BUILD_D3D12
	case RenderSystemD3D12:
		return new D3D12Buffer(type, Device);
		break;
#endif
#if BUILD_VULKAN
	case RenderSystemVulkan:
		return new VKanBuffer(type);
		break;
#endif
	}
	return nullptr;
}

RHICommandList * RHI::CreateCommandList(DeviceContext* Device)
{
	if (Device == nullptr)
	{
		Device = D3D12RHI::GetDefaultDevice();
	}
	switch (instance->CurrentSystem)
	{
#if BUILD_D3D12
	case RenderSystemD3D12:
		return new D3D12CommandList(Device);
		break;
#endif
#if BUILD_VULKAN
	case RenderSystemVulkan:
		return new VKanCommandlist();
		break;
#endif
	}
	return nullptr;
}

bool RHI::BlockCommandlistExec()
{
	return false;
}

bool RHI::AllowCPUAhead()
{
	return true;
}

void RHI::DestoryRHI()
{
	if (instance != nullptr)
	{
		delete instance;
	}
}

#define NOLOADTEX 0
BaseTexture * RHI::CreateTexture(const char * path, DeviceContext* Device)
{
	if (Device == nullptr)
	{
		Device = D3D12RHI::GetDefaultDevice();
	}
#if NOLOADTEX
	//TODO: Default Cube Map!
	if (ImageIO::GetDefaultTexture())
	{
		return ImageIO::GetDefaultTexture();
	}
#endif
	BaseTexture* newtex = nullptr;
	if (ImageIO::CheckIfLoaded(path, &newtex))
	{
		return newtex;
	}
	switch (instance->CurrentSystem)
	{
#if BUILD_D3D12
	case RenderSystemD3D12:
		newtex = new D3D12Texture(Device);
		break;
#endif
	}
	if (!newtex->CreateFromFile(path))
	{
		return ImageIO::GetDefaultTexture();
	}
	ImageIO::RegisterTextureLoad(newtex);
	return newtex;
}

BaseTexture * RHI::CreateTextureWithData(int with, int height, int nChannels, void * data, DeviceContext* Device)
{
	BaseTexture* newtex = nullptr;
	switch (instance->CurrentSystem)
	{
	case RenderSystemD3D12:
		newtex = new D3D12Texture(Device);
		break;
	}
	return newtex;
}

BaseTexture * RHI::CreateNullTexture(DeviceContext * Device)
{
	if (Device == nullptr)
	{
		Device = D3D12RHI::GetDefaultDevice();
	}
	BaseTexture* newtex = nullptr;
	switch (instance->CurrentSystem)
	{

#if BUILD_D3D12
	case RenderSystemD3D12:
		newtex = new D3D12Texture(Device);
		break;
#endif
	}
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
	switch (instance->CurrentSystem)
	{
#if BUILD_D3D12
	case RenderSystemD3D12:
		if (Device == nullptr)
		{
			Device = D3D12RHI::GetDefaultDevice();
		}
		return new D3D12FrameBuffer(Device, Desc);
		break;
#endif
	}
	return nullptr;
}


DeviceContext* RHI::GetDeviceContext(int index)
{
	if (IsD3D12())
	{
		return D3D12RHI::GetDeviceContext(index);
	}
	return nullptr;
}

ShaderProgramBase * RHI::CreateShaderProgam(DeviceContext* Device)
{
	switch (instance->CurrentSystem)
	{
#if BUILD_D3D12
	case RenderSystemD3D12:
		if (Device == nullptr)
		{
			Device = D3D12RHI::GetDefaultDevice();
		}
		return new D3D12Shader(Device);
		break;
#endif
	}
	return nullptr;
}

bool RHI::InitialiseContext(HWND m_hwnd, int w, int h)
{
	if (!m_hwnd)
	{
		return false;
	}
	switch (instance->CurrentSystem)
	{
#if BUILD_D3D12
	case RenderSystemD3D12:
		instance->D3D12Rhi = new D3D12RHI();
		instance->D3D12Rhi->m_height = w;
		instance->D3D12Rhi->m_width = h;
		instance->D3D12Rhi->m_aspectRatio = static_cast<float>(w) / static_cast<float>(h);
		instance->D3D12Rhi->LoadPipeLine();
		instance->D3D12Rhi->LoadAssets();
		break;
#endif
#if BUILD_VULKAN
	case RenderSystemVulkan:
		instance->VulkanRHI = new VKanRHI(instance->m_hinst);
		instance->VulkanRHI->win32HWND = m_hwnd;
		instance->VulkanRHI->InitContext();
		break;
#endif
	}

	return false;
}

void RHI::RHISwapBuffers()
{
	switch (instance->CurrentSystem)
	{
	case RenderSystemD3D12:
		if (D3D12RHI::Instance != nullptr)
		{
			D3D12RHI::Instance->PresentFrame();
		}
		break;
	}
}

void RHI::DestoryContext(HWND hwnd)
{
	switch (instance->CurrentSystem)
	{

	case RenderSystemD3D12:
		instance->D3D12Rhi->DestroyContext();
		break;
	}
}

RHITextureArray * RHI::CreateTextureArray(DeviceContext* Device, int Length)
{
	if (Device == nullptr)
	{
		Device = D3D12RHI::GetDefaultDevice();
	}
	switch (instance->CurrentSystem)
	{
#if BUILD_D3D12
	case RenderSystemD3D12:
		return new D3D12RHITextureArray(Device, Length);
		break;
#endif
	}
	return nullptr;
}