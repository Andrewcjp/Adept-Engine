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
	instance->currentsystem = e;
}

ERenderSystemType RHI::GetType()
{
	return instance->currentsystem;
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

RHIBuffer * RHI::CreateRHIBuffer(RHIBuffer::BufferType type, DeviceContext* Device )
{
	switch (instance->currentsystem)
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
	switch (instance->currentsystem)
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

void RHI::DestoryRHI()
{
	if (instance != nullptr)
	{
		delete instance;
	}
}

BaseTexture * RHI::CreateTexture(const char * path, DeviceContext* Device)
{
	if (Device == nullptr)
	{
		Device = D3D12RHI::GetDefaultDevice();
	}
	switch (instance->currentsystem)
	{
#if BUILD_D3D12
	case RenderSystemD3D12:
		return new D3D12Texture(path, Device);
		break;
#endif
	}
	return nullptr;
}


BaseTexture * RHI::CreateTextureWithData(int with, int height, int nChannels, void * data, DeviceContext* Device)
{
	BaseTexture* newtex = nullptr;
	switch (instance->currentsystem)
	{
	case RenderSystemD3D12:
		newtex = new D3D12Texture(Device);	
		return newtex;
		break;
	}
	return nullptr;
}

Renderable * RHI::CreateMesh(const char * path, ShaderProgramBase* program, bool UseMesh)
{
	///todo asset paths
	std::string accpath = Engine::GetRootDir();
	std::string apath = ("\\asset\\models\\");
	apath.append(path);
	accpath.append(apath);
	AssetManager::RegisterMeshAssetLoad(apath);
	return new Mesh(accpath);
}

FrameBuffer * RHI::CreateFrameBuffer(int width, int height, DeviceContext* Device, float ratio, FrameBuffer::FrameBufferType type,glm::vec4 clearcolour)
{
	switch (instance->currentsystem)
	{
#if BUILD_D3D12
	case RenderSystemD3D12:
		if (Device == nullptr)
		{
			Device = D3D12RHI::GetDefaultDevice();
		}
		D3D12FrameBuffer * ptr = new D3D12FrameBuffer(width, height, Device, ratio, type);
		ptr->InitBuffer(clearcolour);
		return ptr;
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
	switch (instance->currentsystem)
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

void RHI::BindScreenRenderTarget(int width, int height)
{	

}

bool RHI::InitialiseContext(HWND m_hwnd, int w, int h)
{
	if (!m_hwnd)
	{
		return false;
	}
	switch (instance->currentsystem)
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
	switch (instance->currentsystem)
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
	switch (instance->currentsystem)
	{

	case RenderSystemD3D12:
		instance->D3D12Rhi->DestroyContext();
		break;
	}
}
