#include "stdafx.h"
#include "D3D12Window.h"
#if 0
#include "Rendering/Shaders/Shader_Main.h"
#include "RHI/BaseTexture.h"
#include "Core/GameObject.h"
#include "Core/Components/MeshRendererComponent.h"
#include "Rendering/Core/Camera.h"
#include "Rendering/Shaders/Shader_Depth.h"
#include "D3D12Framebuffer.h"

#include "Editor/Editor_Camera.h"
#include "Core/Input.h"
#include "Rendering/Core/ShadowRenderer.h"
#include "D3D12Plane.h"
#include "Rendering/Shaders/ShaderOutput.h"
D3D12Window::D3D12Window()
{
}


D3D12Window::~D3D12Window()
{
}

bool D3D12Window::CreateRenderWindow(int width, int height)
{
	UNUSED_PARAM(Fullscreen);
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		L"RenderWindow", L"OGLWindow", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, width, height, NULL, NULL, hInstance, NULL);
	InitWindow(NULL, m_hwnd, NULL, width, height);
	return true;
}

BOOL D3D12Window::InitWindow(HGLRC, HWND, HDC, int width, int height)
{

	m_width = width;
	m_height = height;
	RHITest = new D3D12RHI();
	RHITest->m_height = m_height;
	RHITest->m_width = m_width;
	RHITest->m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	RHITest->LoadPipeLine();
	RHITest->InitSwapChain();
	mainshader = new Shader_Main();

	Testtexture = RHI::CreateTexture("\\asset\\texture\\house_diffuse.tga");
	Testobj = new GameObject();
	Material* newmat = new Material(RHI::CreateTexture("\\asset\\texture\\grasshillalbedo.png", false));
	Testobj->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("terrainmk2.obj", nullptr), newmat));
	Testobj->GetTransform()->SetPos(glm::vec3(20, 0, 0));
	Testobj->GetTransform()->SetScale(glm::vec3(10));
	Testobj->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	Testobj2 = new GameObject();
	newmat = new Material(RHI::CreateTexture("\\asset\\texture\\house_diffuse.tga", true));
	Testobj2->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("house.obj", nullptr), newmat));
	Testobj2->GetTransform()->SetPos(glm::vec3(0, 2, 0));
	Testobj2->GetTransform()->SetEulerRot(glm::vec3(0, 45, 0));

	cam = new Camera(glm::vec3(0, 5, -15), 70, 1.77f, 0.1f, 1000.0f);
	EditorCam = new Editor_Camera(cam);
	TestShadowcam = new Camera(glm::vec3(0, 5, -10), 70, 1.77f, 0.1f, 1000.0f);
	MainList = ((D3D12Shader*)mainshader->GetShaderProgram())->CreateShaderCommandList();
	RHITest->m_commandList = MainList;
	MainList->SetName(L"Main list");
	int shadowmapsize = 512;
	ddepth = (D3D12FrameBuffer*)RHI::CreateFrameBuffer(shadowmapsize, shadowmapsize, 1, FrameBuffer::Depth);
	Depthshader = new Shader_Depth(light);
	ShadowList = ((D3D12Shader*)Depthshader->GetShaderProgram())->CreateShaderCommandList(1);
	ShadowList->SetName(L"ShadowList");
	//NAME_D3D12_OBJECT(ShadowList);
	ShadowR = new mShadowRenderer();
	debugplane = new D3D12Plane(1);
	Lights.push_back(new Light(TestShadowcam->GetPosition(), 1, Light::Directional));
//	ShadowR->InitShadows(Lights);
	input = new Input(cam, nullptr, m_hwnd, this);
	mainshader->UpdateLightBuffer(Lights);
	output = new ShaderOutput(1280, 720);
	RHITest->ExecSetUpList();
	return 1;
}
void D3D12Window::PrepareRenderData()
{
	mainshader->UpdateUnformBufferEntry(mainshader->CreateUnformBufferEntry(Testobj->GetTransform()), 0);
	mainshader->UpdateUnformBufferEntry(mainshader->CreateUnformBufferEntry(Testobj2->GetTransform()), 1);
}
bool once = true;
void D3D12Window::ShadowPass()
{
	RHITest->PreFrameSetUp(ShadowList, ((D3D12Shader*)Depthshader->GetShaderProgram()));
#if 0
	mainshader->UpdateMV(Lights[0]->DirView, Lights[0]->Projection);
	/*mainshader->UpdateMV(TestShadowcam);*/
	mainshader->BindLightsBuffer(ShadowList);

	ddepth->BindBufferAsRenderTarget(ShadowList);
	ddepth->ClearBuffer(ShadowList);

	mainshader->SetActiveIndex(ShadowList, 1);
	Testobj2->Render(false, ShadowList);
#endif
	std::vector<GameObject*> sh;
	sh.push_back(Testobj2);
	//mainshader->UpdateMV(Lights[0]->DirView, Lights[0]->Projection);
//	ShadowR->RenderShadowMaps(nullptr, Lights, sh, ShadowList, mainshader);

	ShadowList->Close();

	RHITest->ExecList(ShadowList);
}
void D3D12Window::RenderPlane()
{
#if !(_DEBUG)
	return;
#endif
	MainList->SetGraphicsRootSignature(((D3D12Shader*)output->GetShaderProgram())->m_Shader.m_rootSignature);
	MainList->SetPipelineState(((D3D12Shader*)output->GetShaderProgram())->m_Shader.m_pipelineState);
	//debugplane->Render(MainList);
}
void D3D12Window::Render()
{
	float delta = (1.0f / 144.0f);
	input->ProcessInput(delta);
	EditorCam->Update(delta);
	RHITest->Shadowlist = ShadowList;
	PrepareRenderData();
	mainshader->UpdateCBV();
	ShadowPass();
	RHITest->PreFrameSetUp(MainList, ((D3D12Shader*)mainshader->GetShaderProgram()));
	RHITest->PreFrameSwap(MainList);
	RHITest->ClearRenderTarget(MainList);
	mainshader->BindLightsBuffer(MainList);
	mainshader->UpdateMV(cam);
	//copy to gpu

///	ShadowR->BindShadowMaps(MainList);
//	mainshader->SetActiveIndex(RHITest->m_commandList, 0);
	Testobj->Render(false, RHITest->m_commandList);

//	mainshader->SetActiveIndex(RHITest->m_commandList, 1);
	Testobj2->Render(false, RHITest->m_commandList);


	RenderPlane();
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
	input->MouseLBDown(x, y);
	return 0;
}

BOOL D3D12Window::MouseRBUp(int x, int y)
{
	input->MouseLBUp(x, y);
	return 0;
}

BOOL D3D12Window::MouseMove(int x, int y)
{
	input->MouseMove(x, y, 0);
	return 0;
}

BOOL D3D12Window::KeyDown(WPARAM key)
{
	return 0;
}

void D3D12Window::ProcessMenu(WORD command)
{
}
#endif