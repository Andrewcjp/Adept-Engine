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
#include "D3D11\D3D11Mesh.h"
#include "D3D11\D3D11Shader.h"
#include "D3D11\D3D11Texture.h"
#include "OpenGL/OGLFrameBuffer.h"
#include "D3D11/D3D11FrameBuffer.h"
#include "../Rendering/Core/GPUStateCache.h"
#include "../Core/Engine.h"
#include "../D3D12/D3D12Texture.h"
#include "../D3D12/D3D12Mesh.h"
#include "../D3D12/D3D12Shader.h"
#include "../D3D12/D3D12Framebuffer.h"
#include "../Core/Performance/PerfManager.h"
RHI* RHI::instance = nullptr;
RHI::RHI()
{
	DebugD3D11 = false;
}


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

void RHI::DestoryRHI()
{
	if (instance != nullptr)
	{
		delete instance;
	}
}

BaseTexture * RHI::CreateTexture(const char * path, bool istga)
{

	switch (instance->currentsystem)
	{
	case RenderSystemOGL:
		return new OGLTexture(path, istga);
		break;
#if BUILD_D3D11
	case RenderSystemD3D11:
		return new D3D11Texture(path, istga);
		break;
#endif
#if BUILD_D3D12
	case RenderSystemD3D12:
		return new D3D12Texture(path);
		break;
#endif
	}
	return nullptr;
}
void RHI::UnBindUnit(int unit)
{
	switch (instance->currentsystem)
	{
#if BUILD_OPENGL
	case RenderSystemOGL:
		if (GPUStateCache::CheckStateOfUnit(unit, 0) == false)
		{
			glActiveTexture(GL_TEXTURE0 + unit);
			glBindTexture(GL_TEXTURE_2D, 0);
			GPUStateCache::UpdateUnitState(unit, 0);
		}
#endif	
		break;
#if BUILD_D3D11
	case RenderSystemD3D11:

		break;
#endif
	}
}
BaseTexture * RHI::CreateTextureWithData(int with, int height, int nChannels, void * data, TextureType type)
{
	BaseTexture* newtex = nullptr;
	switch (instance->currentsystem)
	{
	case RenderSystemOGL:
		newtex = new OGLTexture();
		newtex->CreateTextureFromData(data, type, with, height, nChannels);
		return newtex;
		break;
#if BUILD_D3D11
	case RenderSystemD3D11:
		newtex = new D3D11Texture();
		newtex->CreateTextureFromData(data, type, with, height, nChannels);
		return newtex;
		break;
#endif
	}
	return nullptr;
}

Renderable * RHI::CreateMesh(const char * path, ShaderProgramBase* program, bool UseMesh)
{
	std::string accpath = Engine::GetRootDir();
	accpath.append("\\asset\\models\\");
	accpath.append(path);
	switch (instance->currentsystem)
	{
	case RenderSystemOGL:
		if (UseMesh)
		{
			return new Mesh(accpath);
		}
		return new OGLMesh(accpath, path);
		break;
#if BUILD_D3D11
	case RenderSystemD3D11:
		return new D3D11Mesh(accpath.c_str(), program);
		break;
#endif
#if BUILD_D3D12
	case RenderSystemD3D12:
		return new D3D12Mesh(accpath.c_str());
		break;
#endif
	}
	return nullptr;
}

FrameBuffer * RHI::CreateFrameBuffer(int width, int height, float ratio, FrameBuffer::FrameBufferType type)
{
	switch (instance->currentsystem)
	{
	case RenderSystemOGL:
		return new OGLFrameBuffer(width, height, ratio, type);
		break;
#if BUILD_D3D11
	case RenderSystemD3D11:
		return new D3D11FrameBuffer(width, height, ratio, type);
		break;
#endif
#if BUILD_D3D12
	case RenderSystemD3D12:
		return new D3D12FrameBuffer(width, height, ratio, type);
		break;
#endif
	}
	return nullptr;

}

ShaderProgramBase * RHI::CreateShaderProgam()
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
		return new D3D12Shader();
		break;
#endif
	}
	return nullptr;
}

void RHI::ClearColour()
{
	switch (instance->currentsystem)
	{
	case RenderSystemOGL:
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		break;
#if BUILD_D3D11
	case RenderSystemD3D11:
		float clearcolour[4] = { 0.0f, 0.0f, 0.2f, 1.0f };
		GetD3DContext()->ClearRenderTargetView(instance->m_backbuffer, clearcolour);
		GetD3DContext()->ClearDepthStencilView(instance->m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0, 0);
		break;
#endif
	}
}

void RHI::ClearDepth()
{
	switch (instance->currentsystem)
	{
	case RenderSystemOGL:
		glClear(GL_DEPTH_BUFFER_BIT);
		break;
	}
}

void RHI::InitRenderState()
{
	switch (instance->currentsystem)
	{
	case RenderSystemOGL:
		glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL);
		glEnable(GL_MULTISAMPLE);
		break;
	}

}

void RHI::SetDepthMaskState(bool state)
{
	switch (instance->currentsystem)
	{
	case RenderSystemOGL:
		glDepthMask(state ? GL_TRUE : GL_FALSE);
		break;
	}
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

bool RHI::InialiseContext(HWND m_hwnd, int w, int h)
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
	case RenderSystemOGL:
		SwapBuffers(instance->m_hdc);
		break;
#if BUILD_D3D11
	case RenderSystemD3D11:
		instance->m_swapChain->Present(0, 0);
		break;
#endif
	}
}
void RHI::DestoryContext(HWND hwnd)
{
	switch (instance->currentsystem)
	{
	case RenderSystemOGL:
		instance->DestroyOGLContext(hwnd);
		break;
#if BUILD_D3D11
	case RenderSystemD3D11:
		instance->DestroyD3DDevice();
		break;
#endif

	}
}

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
#if BUILD_OPENGL
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
#if BUILD_D3D11
void RHI::CreateDepth()
{
	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = mwidth;
	descDepth.Height = mheight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	HRESULT  result = m_dxDev->CreateTexture2D(&descDepth, nullptr, &m_depthStencil);
	if (FAILED(result))
	{

	}


	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	result = m_dxDev->CreateDepthStencilView(m_depthStencil, &descDSV, &m_depthStencilView);
}
void RHI::ResizeContext(int width, int height)
{
	instance->mwidth = width;
	instance->mheight = height;
	if (instance->m_depthStencil != nullptr)
	{
		instance->m_depthStencil->Release();
		instance->m_depthStencilView->Release();
	}
	instance->CreateDepth();
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<FLOAT>(width);
	viewport.Height = static_cast<FLOAT>(height);
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;


	RHI::GetD3DContext()->RSSetViewports(1, &viewport);
}
BOOL RHI::InitD3DDevice(HWND hWnd, int w, int h)
{
	mwidth = w;
	mheight = h;

	DXGI_SWAP_CHAIN_DESC dxscd;

	ZeroMemory(&dxscd, sizeof(DXGI_SWAP_CHAIN_DESC));

	dxscd.BufferCount = 1;
	dxscd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxscd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxscd.OutputWindow = hWnd;
	dxscd.SampleDesc.Count = 1;
	dxscd.Windowed = TRUE;
	//
	HRESULT result = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE, NULL,
		(DebugD3D11 ? D3D11_CREATE_DEVICE_DEBUG : NULL),
		NULL, NULL, D3D11_SDK_VERSION, &dxscd, &m_swapChain, &m_dxDev, NULL, &m_dxContext);

	if (result != S_OK)
		return FALSE;

	//Create a back buffer for double buffering
	ID3D11Texture2D *pBackBufferSurface;
	m_swapChain->GetBuffer(0, __uuidof(pBackBufferSurface), (void**)&pBackBufferSurface);

	m_dxDev->CreateRenderTargetView(pBackBufferSurface, NULL, &m_backbuffer);
	pBackBufferSurface->Release();

	CreateDepth();


	m_dxContext->OMSetRenderTargets(1, &m_backbuffer, m_depthStencilView);

	//Init shader
	//m_shaderProgram = new D3D11ShaderProgram(m_dxDev, m_dxContext);
//	m_shaderProgram->AttachAndCompileShaderFromFile(L"../asset/shader/hlsl/basic_vs.hlsl", SHADER_VERTEX);
//	m_shaderProgram->AttachAndCompileShaderFromFile(L"../asset/shader/hlsl/basic_fs.hlsl", SHADER_FRAGMENT);

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	m_dxDev->CreateBuffer(&bd, NULL, &m_constantBuffer);

	//m_mesh = new D3D11Mesh(L"../asset/models/house.obj", m_shaderProgram);

	//Create a texture sampler state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 4;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_dxDev->CreateSamplerState(&sampDesc, &m_texSamplerDefaultState);


	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_dxDev->CreateSamplerState(&sampDesc, &m_texSamplerFBState);
	/*m_texture = new D3D11Texture();
	m_texture->CreateTextureFromFile(m_dxDev, "../asset/texture/house_diffuse.tga");

	m_mesh->SetTexture(m_texture);*/


	return TRUE;
}
BOOL RHI::DestroyD3DDevice()
{

	m_swapChain->Release();
	m_backbuffer->Release();
	m_depthStencil->Release();
	m_depthStencilView->Release();
	m_constantBuffer->Release();
	m_texSamplerDefaultState->Release();
	m_dxContext->Release();
	m_dxDev->Release();

	return TRUE;
}
#endif