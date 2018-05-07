#include "RHI.h"
#include "BaseTexture.h"
#include "OpenGL/OGLTexture.h"
#include "../Rendering/Core/Renderable.h"
#include "OpenGL/OGLMesh.h"
#include "../Rendering/Core/Mesh.h"
#include "RHI/ShaderBase.h"
#include "OpenGL/OGLShader.h"
#include "ShaderProgramBase.h"
#include "OpenGL/OGLShaderProgram.h"
#include <GLEW\GL\glew.h>
#include "../Core/Performance/PerfManager.h"
#include "../Core/Assets/AssetManager.h"
#include "../Rendering/Core/Mesh.h"
#include "OpenGL/OGLFrameBuffer.h"
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
#include "../OpenGL/OGLCommandList.h"
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
#if BUILD_OPENGL
	case RenderSystemOGL:
		return new OGLBuffer(type);
		break;
#endif
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
#if BUILD_OPENGL
	case RenderSystemOGL:
		return new OGLCommandList();
		break;
#endif
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
void RHI::UnBindUnit(int unit)
{
}

BaseTexture * RHI::CreateTextureWithData(int with, int height, int nChannels, void * data, DeviceContext* Device)
{
	BaseTexture* newtex = nullptr;
	switch (instance->currentsystem)
	{
	/*case RenderSystemOGL:
		newtex = new OGLTexture();
		newtex->CreateTextureFromData(data, type, with, height, nChannels);
		return newtex;
		break;*/
	case RenderSystemD3D12:
		newtex = new D3D12Texture(Device);
		//newtex->CreateTextureFromData(data, type, with, height, nChannels);
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

	/*switch (instance->currentsystem)
	{
	case RenderSystemOGL:
		if (UseMesh)
		{
			return new Mesh(accpath);
		}
		return new OGLMesh(accpath, path);
		break;
#if BUILD_D3D12
	case RenderSystemD3D12:
		return new D3D12Mesh(accpath.c_str());
		break;
#endif
	}*/
	return nullptr;
}

FrameBuffer * RHI::CreateFrameBuffer(int width, int height, DeviceContext* Device, float ratio, FrameBuffer::FrameBufferType type,glm::vec4 clearcolour)
{
	switch (instance->currentsystem)
	{
#if BUILD_OPENGL
	case RenderSystemOGL:
		return new OGLFrameBuffer(width, height, ratio, type);
		break;
#endif
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
	case RenderSystemOGL:
		return new OGLShaderProgram();
		break;
#if BUILD_D3D11
	case RenderSystemD3D11:
		return new D3D11ShaderProgram(instance->m_dxDev, instance->m_dxContext);
		break;
#endif
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
	switch (instance->currentsystem)
	{
	case RenderSystemOGL:
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		break;
#if BUILD_D3D11
	case RenderSystemD3D11:
		RHI::GetD3DContext()->OMSetRenderTargets(1, &instance->m_backbuffer, instance->m_depthStencilView);

		D3D11_VIEWPORT viewport;

		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<FLOAT>(width);
		viewport.Height = static_cast<FLOAT>(height);
		viewport.MaxDepth = 1.0f;
		viewport.MinDepth = 0.0f;

		RHI::GetD3DContext()->RSSetViewports(1, &viewport);
		break;
#endif
	}

}

bool RHI::InitialiseContext(HWND m_hwnd, int w, int h)
{
	if (!m_hwnd)
	{
		return false;
	}
	switch (instance->currentsystem)
	{
#if BUILD_OPENGL
	case RenderSystemOGL:
		instance->m_hdc = GetDC(m_hwnd);
		instance->m_hglrc = instance->CreateOGLContext(instance->m_hdc);
		if (instance->m_hglrc == nullptr)
		{
			return false;
		}
		break;
#endif
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
#if BUILD_D3D11
	case RenderSystemD3D11:
		instance->InitD3DDevice(m_hwnd, w, h);
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
#if BUILD_OPENGL
BOOL RHI::DestroyOGLContext(HWND hwnd)
{
	if (m_hglrc)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_hglrc);
		m_hglrc = NULL;
	}

	if (!ReleaseDC(hwnd, m_hdc))
		return FALSE;
	m_hdc = NULL;
	return TRUE;
}

#define MAJOR 3
#define MINOR 3
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB			0x9126
#define	WGL_CONTEXT_CORE_PROFILE_BIT_ARB	0x00000001
int attriblist[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, MAJOR, WGL_CONTEXT_MINOR_VERSION_ARB, MINOR, WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 0, 0 };

HGLRC wglCreateContextAttribsARB(HDC hDC, HGLRC hShareContext, const int *attribList)
{
	typedef HGLRC(APIENTRY * PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int *attribList);
	static PFNWGLCREATECONTEXTATTRIBSARBPROC pfnCreateContextAttribsARB = 0;

	HGLRC hContext = 0;
	HGLRC hCurrentContext = wglGetCurrentContext();

	if (!hCurrentContext)
	{
		hCurrentContext = wglCreateContext(hDC);
		if (hCurrentContext == nullptr)
		{
			return 0;
		}

		if (!wglMakeCurrent(hDC, hCurrentContext))
		{
			wglDeleteContext(hCurrentContext);
			return 0;
		}

		pfnCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));

		if (pfnCreateContextAttribsARB)
			hContext = pfnCreateContextAttribsARB(hDC, hShareContext, attribList);

		wglMakeCurrent(hDC, 0);
		wglDeleteContext(hCurrentContext);
	}
	else
	{
		if (!wglMakeCurrent(hDC, hCurrentContext))
			return 0;

		pfnCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
		if (pfnCreateContextAttribsARB)
			hContext = pfnCreateContextAttribsARB(hDC, hShareContext, attribList);
	}

	return hContext;
}

#define USEALTCONTEXT 0
HGLRC RHI::CreateOGLContext(HDC hdc)
{
	unsigned int pixelformat;
	HGLRC hglrc;

	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Support Window
		PFD_SUPPORT_OPENGL |						// Format Support OpenGL
		PFD_DOUBLEBUFFER ,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request RGBA Format
		24,											// Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits mask
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// Z-Buffer depth
		0,											// Stencil Buffer
		0,											// Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	long time = PerfManager::get_nanos();
	if (!(pixelformat = ChoosePixelFormat(hdc, &pfd)))
	{
		return 0;
	}


	if (!SetPixelFormat(hdc, pixelformat, &pfd))
	{
		return 0;
	}

#if USEALTCONTEXT


	if (!(hglrc = wglCreateContextAttribsARB(hdc, 0, attriblist)))
	{
		return 0;
	}
#else
	if (!(hglrc = wglCreateContext(hdc)))
	{
		return 0;
	}
#endif
	if (!wglMakeCurrent(hdc, hglrc))
	{
		return 0;
	}
	std::cout << "ChoosePixelFormat took " << fabs(((float)(PerfManager::get_nanos() - time)) / 1e6f) << "ms " << std::endl;
	std::cout << "OGL Context Created" << std::endl;
	//MessageBoxA(0, (char*)glGetString(GL_VERSION), "OPENGL VERSION", 0);
	return hglrc;
}
#endif
