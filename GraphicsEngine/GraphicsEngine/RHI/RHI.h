#pragma once
#include "EngineGlobals.h"
#include <Windows.h>
//#include <d3d11_1.h>
//#include <d3d12.h>
//#include <DirectXMath.h>
#include "../Rendering/Core/FrameBuffer.h"
#ifdef _DEBUG
#define D3DEnsure(hr) if(((HRESULT)(hr)) < 0){  __debugbreak();}
#else 
#define D3DEnsure(hr) if(((HRESULT)(hr)) < 0){ printf("D3D ensure failed: %d",hr);  }
#endif
#include "RHICommandList.h"
//using namespace DirectX;
class BaseTexture;
class Renderable;
class ShaderProgramBase;
class FrameBuffer;
class RHI
{
public:
	enum TextureType
	{
		Normal,
		Text
	};
	RHI();
	~RHI();
	static RHI* instance;

	static void InitRHI(ERenderSystemType e);
	static void DestoryRHI();
	static BaseTexture* CreateTexture(const char * path, bool istga = false);
	static void UnBindUnit(int unit);
	static BaseTexture* CreateTextureWithData(int with, int height,int nChannels, void * data, TextureType type = Normal);
	static Renderable * CreateMesh(const char * path, ShaderProgramBase* program, bool UseMesh = false);
	static FrameBuffer* CreateFrameBuffer(int width, int height, float ratio = 1.0f, FrameBuffer::FrameBufferType type = FrameBuffer::FrameBufferType::ColourDepth);
	static ShaderProgramBase* CreateShaderProgam();
	static void ClearColour();
	static void ClearDepth();
	static void InitRenderState();
	static void SetDepthMaskState(bool state);
	static void BindScreenRenderTarget(int mwidth, int height);
	static bool InialiseContext(HWND m_hwnd, int w, int h);
	static void RHISwapBuffers();
	static void DestoryContext(HWND hwnd);
	static ERenderSystemType GetType();
	static bool IsOpenGL();
	static bool IsD3D12();
	static bool SupportsThreading();
	static RHIBuffer* CreateRHIBuffer(RHIBuffer::BufferType type);
	static RHICommandList* CreateCommandList();
#if BUILD_D3D11
	static ID3D11Device* GetD3DDevice()
	{
		return instance->m_dxDev;
	}
	static ID3D11DeviceContext* GetD3DContext()
	{
		return instance->m_dxContext;
	}
	static void ResizeContext(int width, int height);
	ID3D11SamplerState			*m_texSamplerDefaultState;
	ID3D11SamplerState			*m_texSamplerFBState;
	ID3D11Device				*m_dxDev;
	ID3D11DeviceContext			*m_dxContext;
	ID3D11RenderTargetView		*m_backbuffer;
	ID3D11Texture2D				*m_depthStencil;
	ID3D11DepthStencilView		*m_depthStencilView;

	IDXGISwapChain				*m_swapChain;
	ID3D11Buffer				*m_constantBuffer;
#endif
private:
	BOOL DestroyOGLContext(HWND h);
	HGLRC CreateOGLContext(HDC hdc);
	void CreateDepth();	
	BOOL InitD3DDevice(HWND hWnd, int w, int h);
	BOOL DestroyD3DDevice();
	ERenderSystemType currentsystem;
	//----------------------------------
	//opengl only
	HDC			m_hdc;
	HGLRC       m_hglrc;
	//------------------------------------
	//Directx only
#if BUILD_D3D11
	int mwidth = 0;
	int mheight = 0;
#endif

	
};

