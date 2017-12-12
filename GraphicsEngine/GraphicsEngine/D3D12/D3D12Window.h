#pragma once
#include "../RHI/RenderWindow.h"
#include "../D3D12/D3D12RHI.h"
#include "../EngineGlobals.h"
class D3D12Window : public RenderWindow
{
public:
	D3D12Window();
	~D3D12Window();

	// Inherited via RenderWindow
	virtual bool CreateRenderWindow(HINSTANCE hInstance, int width, int height, bool Fullscreen = false) override;
	virtual BOOL InitWindow(HGLRC hglrc, HWND hwnd, HDC hdc, int width, int height) override;
	void PrepareRenderData();
	void ShadowPass();
	void RenderPlane();
	virtual void Render() override;
	virtual void Resize(int width, int height) override;
	virtual void DestroyRenderWindow() override;
	virtual BOOL MouseLBDown(int x, int y) override;
	virtual BOOL MouseLBUp(int x, int y) override;
	virtual BOOL MouseRBDown(int x, int y) override;
	virtual BOOL MouseRBUp(int x, int y) override;
	virtual BOOL MouseMove(int x, int y) override;
	virtual BOOL KeyDown(WPARAM key) override;
private:
	D3D12RHI* RHITest;
	class Shader_Main* mainshader;
	class BaseTexture* Testtexture;
	class GameObject* Testobj;
	class GameObject* Testobj2;
	class Camera* cam;
	class Camera* TestShadowcam;//temp will be removed!
	class Shader_Depth* Depthshader;
	class Light* light;
	class D3D12FrameBuffer* ddepth;
	class Editor_Camera* EditorCam;
	class Input* input;
	class ShadowRenderer* ShadowR;
	class D3D12Plane* debugplane;
	class ShaderOutput* output;
	CommandListDef* MainList;
	CommandListDef* ShadowList;
	std::vector<Light*> Lights;

	// Inherited via RenderWindow
	virtual void ProcessMenu(WORD command) override;
};

