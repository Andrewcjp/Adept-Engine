#include "stdafx.h"
#include "D3D12Window.h"
#include "../Rendering/Shaders/Shader_Main.h"
#include "../RHI/BaseTexture.h"
#include "../Core/GameObject.h"
#include "../Core/Components/MeshRendererComponent.h"
#include "../Rendering/Core/Camera.h"
#include "../Rendering/Shaders/Shader_Depth.h"
#include "D3D12Framebuffer.h"
#include "../EngineGlobals.h"
D3D12Window::D3D12Window()
{
}


D3D12Window::~D3D12Window()
{
}

bool D3D12Window::CreateRenderWindow(HINSTANCE hInstance, int width, int height, bool Fullscreen)
{
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		L"RenderWindow", L"OGLWindow", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, width, height, NULL, NULL, hInstance, NULL);
	InitWindow(NULL, m_hwnd, NULL, width, height);
	return true;
}

BOOL D3D12Window::InitWindow(HGLRC hglrc, HWND hwnd, HDC hdc, int width, int height)
{

	m_width = width;
	m_height = height;
	RHITest = new D3D12RHI();
	RHITest->m_height = m_height;
	RHITest->m_width = m_width;
	RHITest->m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	RHITest->LoadPipeLine();
	RHITest->LoadAssets();
	mainshader = new Shader_Main();

	Testtexture = RHI::CreateTexture("house_diffuse.tga");
	Testobj = new GameObject();
	Material* newmat = new Material(RHI::CreateTexture("house_diffuse.tga", true));
	Testobj->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("house.obj", nullptr), newmat));
	Testobj->GetTransform()->SetPos(glm::vec3(0, 0, 0));
	Testobj->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	Testobj2 = new GameObject();
	newmat = new Material(RHI::CreateTexture("house_diffuse.tga", true));
	Testobj2->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("house.obj", nullptr), newmat));
	Testobj2->GetTransform()->SetPos(glm::vec3(0, 10, 0));
	Testobj2->GetTransform()->SetEulerRot(glm::vec3(0, 45, 0));

	cam = new Camera(glm::vec3(0, 10, -10), 70, 1.77f, 0.1f, 1000.0f);
	TestShadowcam = new Camera(glm::vec3(0, 0, -10), 70, 1.77f, 0.1f, 1000.0f);
	MainList = ((D3D12Shader*)mainshader->GetShaderProgram())->CreateShaderCommandList();
	RHITest->m_commandList = MainList;
	ddepth = (D3D12FrameBuffer*)RHI::CreateFrameBuffer(m_width, m_height, 1, FrameBuffer::Depth);
	Depthshader = new Shader_Depth(light);
	ShadowList = ((D3D12Shader*)Depthshader->GetShaderProgram())->CreateShaderCommandList();
	//NAME_D3D12_OBJECT(ShadowList);
	std::vector<Light*> Lights;
	Lights.push_back(new Light(glm::vec3(0, 0, 0), 1));

	mainshader->UpdateLightBuffer(Lights);
	RHITest->ExecSetUpList();
	return 1;
}
void D3D12Window::PrepareRenderData()
{
	mainshader->UpdateUnformBufferEntry(mainshader->CreateUnformBufferEntry(Testobj->GetTransform(), cam), 0);
	mainshader->UpdateUnformBufferEntry(mainshader->CreateUnformBufferEntry(Testobj2->GetTransform(), cam), 1);
}
void D3D12Window::ShadowPass()
{
	RHITest->PreFrameSetUp(ShadowList, ((D3D12Shader*)Depthshader->GetShaderProgram()));
	//RHITest->RenderToScreen(ShadowList);
//	RHITest->ClearRenderTarget(ShadowList);
	
	mainshader->UpdateMV(TestShadowcam);
	mainshader->BindLightsBuffer(ShadowList);
	ddepth->ClearBuffer(ShadowList);
	ddepth->BindBufferAsRenderTarget(ShadowList);

	mainshader->SetActiveIndex(ShadowList, 0);
	Testobj->Render(true, ShadowList);
	mainshader->SetActiveIndex(ShadowList, 1);
	Testobj2->Render(true, ShadowList);
	ShadowList->Close();

	RHITest->ExecList(ShadowList);
}
void D3D12Window::Render()
{
	PrepareRenderData();
	mainshader->UpdateCBV();
	ShadowPass();
	RHITest->PreFrameSetUp(MainList, ((D3D12Shader*)mainshader->GetShaderProgram()));
	RHITest->PreFrameSwap(MainList);
	RHITest->ClearRenderTarget(MainList);
	mainshader->BindLightsBuffer(MainList);
	mainshader->UpdateMV(cam);
	//copy to gpu
	ddepth->BindBufferToTexture(MainList);
	mainshader->SetActiveIndex(RHITest->m_commandList, 0);
	Testobj->Render(false, RHITest->m_commandList);

	mainshader->SetActiveIndex(RHITest->m_commandList, 1);
	Testobj2->Render(false, RHITest->m_commandList);
	//RHITest->PopulateCommandList();
	//ddepth->UnBind(MainList);
	RHITest->PostFrame(RHITest->m_commandList);
	RHITest->PresentFrame();
}

void D3D12Window::Resize(int width, int height)
{

}

void D3D12Window::DestroyRenderWindow()
{
	RHITest->OnDestroy();
	delete RHITest;
}

BOOL D3D12Window::MouseLBDown(int x, int y)
{
	return 0;
}

BOOL D3D12Window::MouseLBUp(int x, int y)
{
	return 0;
}

BOOL D3D12Window::MouseRBDown(int x, int y)
{
	return 0;
}

BOOL D3D12Window::MouseRBUp(int x, int y)
{
	return 0;
}

BOOL D3D12Window::MouseMove(int x, int y)
{
	return 0;
}

BOOL D3D12Window::KeyDown(WPARAM key)
{
	return 0;
}
