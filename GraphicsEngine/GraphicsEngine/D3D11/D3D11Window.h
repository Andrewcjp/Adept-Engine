#pragma once
#include "EngineGlobals.h"
#if BUILD_D3D11
#include <Windows.h>
#include <WindowsX.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include "RHI/RenderWindow.h"
#include "../Core/GameObject.h"
#include "D3D11Shader.h"
#include "D3D11Cube.h"
#include "D3D11Mesh.h"
#include "D3D11Texture.h"
#include "../Rendering/Core/Camera.h"
#include "../Core/Input.h"
#include "glm\glm.hpp"
#include "D3D11/D3D11FrameBuffer.h"
//We will use some Direct X Maths utilities
using namespace DirectX;
#include "D3D11Plane.h"
#include "Rendering/Shaders/ShaderOutput.h"
#include "../Rendering/Core/ShadowRenderer.h"
#include "../Rendering/Core/Light.h"
__declspec(align(16)) class D3D11Window : public RenderWindow
{
	private:
		struct ConstantBuffer
		{
			glm::mat4		m_worldMat;
			glm::mat4		m_viewMat;
			glm::mat4		m_projection;
		};

	private:
		ConstantBuffer				m_cbuffer;
		glm::mat4					m_world;
		glm::mat4					m_view;
		glm::mat4					m_proj;
		float						m_euler[3];
		
		//ID3D11Device				*m_dxDev;
		//ID3D11DeviceContext			*m_dxContext;
		//ID3D11RenderTargetView		*m_backbuffer;
		//ID3D11Texture2D				*m_depthStencil;
		//ID3D11DepthStencilView		*m_depthStencilView;
		//ID3D11SamplerState			*m_texSamplerDefaultState;
		//IDXGISwapChain				*m_swapChain;
		//ID3D11Buffer				*m_constantBuffer;
		D3D11ShaderProgram			*m_shaderProgram;
		D3D11ShaderProgram			*m_output;
		Renderable					*m_mesh;
		BaseTexture				*m_texture;
		D3D11FrameBuffer* Framebuffer;
		D3D11Mesh* plane;
		D3D11Plane* cube;
		D3D11Cube* acube;
		Input*						m_input;
		Camera*						m_Cam;
		ShaderOutput*		out;
		GameObject* testgo = nullptr;
		GameObject* shadow = nullptr;
		ShadowRenderer* Shadows = nullptr;
		Shader_Depth*	depthtestshader = nullptr;
		FrameBuffer* dephtestbuffer;
		std::vector<Light*> Lights;
		std::vector<GameObject*> Objects;
		float deltatime = 0;
		long lasttime = 0;
		float FrameRateTarget = 60.0f;
		float Targettime = 0;
		float Acculilatedtime = 0;
protected:

		BOOL InitD3DDevice (HWND hWnd);
		BOOL DestroyD3DDevice();
		void InitD3D11State();

	public:
					D3D11Window();
					D3D11Window(HINSTANCE hInstance, int width, int height);
					~D3D11Window();

		virtual bool CreateRenderWindow(HINSTANCE hInstance, int width, int height) override;
		void		Render();
		void		Resize( int width, int height );
		void		DestroyRenderWindow();

		//We need to make sure the memory allocation is 16 byte aligned
		//since SSE intrinsics are used in Direct X math utilities.
		void*		operator new (size_t size)
		{
			return _aligned_malloc( size, 16 );
		}

		void		operator delete( void* p )
		{
			_aligned_free( p );
		}

		BOOL		MouseLBDown ( int x, int y );
		BOOL		MouseLBUp ( int x, int y );
		BOOL		MouseMove ( int x, int y );
		BOOL KeyDown(WPARAM key);
		BOOL KeyUp(WPARAM key);
		BOOL KeyHeld(WPARAM key);

		// Inherited via RenderWindow
		
		virtual BOOL InitWindow(HGLRC hglrc, HWND hwnd, HDC hdc, int width, int height) override;

		// Inherited via RenderWindow
		virtual BOOL MouseRBDown(int x, int y) override;
		virtual BOOL MouseRBUp(int x, int y) override;

		// Inherited via RenderWindow
		virtual void AddPhysObj(GameObject * go) override;
};
#endif