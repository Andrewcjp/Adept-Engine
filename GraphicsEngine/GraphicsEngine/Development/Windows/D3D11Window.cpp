#include "D3D11Window.h"

#if BUILD_D3D11
#include "RHI/RHI.h"
#include "../Core/Engine.h" 
#include "../Core/Components/MeshRendererComponent.h"
#include "../Rendering/Shaders/Shader_Main.h"
#include "../Core/Performance/PerfManager.h"
D3D11Window::D3D11Window()
{
	m_shaderProgram = NULL;

	m_euler[0] = m_euler[1] = m_euler[2] = 0.0f;
}

D3D11Window::D3D11Window(HINSTANCE , int , int )
{
	//InitWindow(hInstance, width, height);	
	m_euler[0] = m_euler[1] = m_euler[2] = 0.0f;
}

D3D11Window::~D3D11Window()
{
	if (m_shaderProgram) delete m_shaderProgram;

	delete m_mesh;
	delete m_texture;

	DestroyD3DDevice();
}

BOOL D3D11Window::InitD3DDevice(HWND hWnd)
{
	RHI::InitialiseContext(hWnd, m_width, m_height);
	//Init shader
	//m_shaderProgram = new D3D11ShaderProgram(m_dxDev, m_dxContext);
	m_shaderProgram = (D3D11ShaderProgram*)RHI::CreateShaderProgam();
	m_shaderProgram->AttachAndCompileShaderFromFile("Main_vs", SHADER_VERTEX);
	m_shaderProgram->AttachAndCompileShaderFromFile("Main_fs", SHADER_FRAGMENT);


	

	m_mesh = RHI::CreateMesh("house.obj", m_shaderProgram);
	/*m_texture = new D3D11Texture();
	m_texture->CreateTextureFromFile( m_dxDev, "../asset/texture/house_diffuse.tga" );*/
	m_texture = RHI::CreateTexture("house_diffuse.tga", true);
	m_mesh->SetTexture(m_texture);

	Framebuffer = new D3D11FrameBuffer(m_width, m_height, 1);

	m_output = (D3D11ShaderProgram*)RHI::CreateShaderProgam();
	m_output->AttachAndCompileShaderFromFile("Pass_vs", SHADER_VERTEX);
	m_output->AttachAndCompileShaderFromFile("Pass_fs", SHADER_FRAGMENT);
	//plane = new D3D11Mesh("../asset/models/RenderPlane.obj", m_output);
	plane = (D3D11Mesh*)RHI::CreateMesh("RenderPlane.obj", m_shaderProgram);// new D3D11Mesh("RenderPlane.obj", m_output);
	cube = new D3D11Plane(m_output);
	acube = new D3D11Cube(m_output);
	out = new ShaderOutput(m_width, m_height);

	testgo = new GameObject("Terrain");


	Material* mat = new Material(RHI::CreateTexture("house_diffuse.tga"));
	//mat->NormalMap = new OGLTexture("../asset/texture/Normal.tga");
	//	mat->DisplacementMap = new OGLTexture("../asset/texture/bricks2_disp.jpg");
	testgo->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("terrainmk2.obj", m_shaderProgram), mat));
//	testgo->SetMaterial(mat);
//	testgo->SetMesh(RHI::CreateMesh("../asset/models/terrainmk2.obj", m_shaderProgram));
	//	go->GetMat()->SetShadow(false);
	//	m_mesh->position = glm::vec3(0, 0, -10);
	testgo->GetTransform()->SetPos(glm::vec3(10, 0, 0));
	testgo->GetTransform()->SetEulerRot(glm::vec3(0,45, 0));
	testgo->GetTransform()->SetScale(glm::vec3(2));
	Objects.push_back(testgo);
	shadow = new GameObject("cube");
	shadow->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("house.obj", m_shaderProgram), new Material(RHI::CreateTexture("house_diffuse.tga", true))));
	
	shadow->GetTransform()->SetPos(glm::vec3(0, 10, 0));
	shadow->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	shadow->GetTransform()->SetScale(glm::vec3(2));
	Objects.push_back(shadow);
	return TRUE;
}

BOOL D3D11Window::DestroyD3DDevice()
{
	/*m_swapChain->Release();
	m_backbuffer->Release();
	m_depthStencil->Release();
	m_depthStencilView->Release();
	m_constantBuffer->Release();
	m_texSamplerDefaultState->Release();
	m_dxContext->Release();
	m_dxDev->Release();*/
	RHI::DestoryContext(nullptr);

	return TRUE;
}

bool D3D11Window::CreateRenderWindow(HINSTANCE hInstance, int width, int height, bool )
{
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		L"RenderWindow", L"D3D11Window", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, width, height, NULL, NULL, hInstance, NULL);

	if (!m_hwnd)
		return FALSE;

	m_width = width;
	m_height = height;

	InitD3DDevice(m_hwnd);
	m_Cam = new Camera(glm::vec3(0, 0, -10.0f), 70.0f, static_cast<float>(width / height), 0.1f, 1000.0f);
	m_input = new Input(m_Cam, NULL, m_hwnd, NULL);
	Shadows = new mShadowRenderer();
	dephtestbuffer = new D3D11FrameBuffer(1024, 1024, 1, FrameBuffer::Depth);
	Light* l = new Light(glm::vec3(0, 50, 30), 10, Light::Directional);
	l->SetDirection(l->GetPosition() - glm::vec3(0, 0, 0));
	Lights.push_back(l);
	Shadows->InitShadows(Lights);
	depthtestshader = new Shader_Depth(l,false);
	mainshader = new Shader_Main();
	FrameRateTarget = 120.0f;
	Targettime = 1.0f / (FrameRateTarget*1.1f);
	return TRUE;
}
XMVECTOR todxvec(glm::vec3 v)
{
	return XMVectorSet(v.x, v.y, v.z, 0);
}
void D3D11Window::Render()
{
	deltatime = (float)(PerfManager::get_nanos() - lasttime) / 1.0e9f;//in ms
	lasttime = PerfManager::get_nanos();
	m_input->ProcessInput(deltatime);
	Acculilatedtime += deltatime;
	if (Targettime > Acculilatedtime)
	{
		return;
	}
	else
	{
		Acculilatedtime = 0;
	}
	

	float clearcolour[4] = { 0.0f, 0.0f, 0.2f, 1.0f };

	float inc = -XM_PIDIV4*0.0005f;

	m_euler[0] = -XM_PIDIV4*0.4;
	m_euler[1] = m_euler[1] > XM_2PI ? 0 : m_euler[1] + inc;
	m_euler[2] = 0.0;



	//Remember, D3D11 uses a LEFT HANDED system
	//Setting up view matrix
	glm::vec3 eye = glm::vec3(0.0f, 0.0f, -10.0f);
	glm::vec3 at = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	//m_view = glm::lookAtLH(Lights[0]->GetPosition(), at, up);
	//	m_view = XMMatrixLookAtLH(todxvec(m_Cam->GetPosition()), at, up);
	//m_view = XMMatrixLookAtLH(eye, at, up);
	m_view = m_Cam->GetView();
	m_world = glm::mat4();

	//------------------------
	//Setting world matrix 

	//add some idle rotations using euler angles;

	//m_world = XMMatrixRotationY(m_euler[2])*XMMatrixRotationY(m_euler[1])*XMMatrixRotationX(m_euler[0]);
	//m_world = glm::rotate(m_euler[1], glm::vec3(0, 1, 0));
	//copy the matrices to constant buffer for shader use
	//----------------------------------------------------
	RHI::GetD3DContext()->ClearRenderTargetView(RHI::instance->m_backbuffer, clearcolour);
	RHI::GetD3DContext()->ClearDepthStencilView(RHI::instance->m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0, 0);
	//----------------------------------------------------

	//----------------------------------------------------

	//depthtestshader->SetShaderActive();
	//Shadows->RenderShadowMaps(m_Cam, Lights, Objects);
	//dephtestbuffer->BindBufferAsRenderTarget();
	//dephtestbuffer->ClearBuffer();
	//m_mesh->Render();
	//testgo->Render();
	////---------------------------------
	//m_cbuffer.m_worldMat = m_world;
	//m_cbuffer.m_viewMat = m_view;
	//m_cbuffer.m_projection = m_Cam->GetProjection();
	//RHI::GetD3DContext()->UpdateSubresource(RHI::instance->m_constantBuffer, 0, NULL, &m_cbuffer, 0, 0);
	//RHI::GetD3DContext()->VSSetConstantBuffers(0, 1, &RHI::instance->m_constantBuffer);
	////------------------------

	//m_shaderProgram->ActivateShaderProgram();
	////Shadows->BindShadowMaps();
	//m_texture->Bind(0);
	//
	//m_mesh->Render();

	//m_cbuffer.m_worldMat = testgo->GetTransform()->GetModel();
	//RHI::GetD3DContext()->UpdateSubresource(RHI::instance->m_constantBuffer, 0, NULL, &m_cbuffer, 0, 0);
	//RHI::GetD3DContext()->VSSetConstantBuffers(0, 1, &RHI::instance->m_constantBuffer);
	//testgo->Render();
	Framebuffer->BindBufferAsRenderTarget();
	Framebuffer->ClearBuffer();
	for (int i = 0; i < Objects.size(); i++)
	{
		mainshader->SetShaderActive();
		mainshader->UpdateUniforms(Objects[i]->GetTransform(), m_Cam, Lights);
		Objects[i]->Render();
	}


	RHI::BindScreenRenderTarget(m_width, m_height);
	out->SetShaderActive();	
	out->UpdateUniforms(nullptr, m_Cam);
	Framebuffer->BindToTextureUnit(0);	
	out->RenderPlane();	
	Framebuffer->UnBind();
	RHI::RHISwapBuffers();

}

void D3D11Window::Resize(int width, int height)
{
	if (RHI::GetD3DContext() != nullptr)
	{
		/*D3D11_VIEWPORT viewport;

		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<FLOAT>(width);
		viewport.Height = static_cast<FLOAT>(height);
		viewport.MaxDepth = 1.0f;
		viewport.MinDepth = 0.0f;

		float aspectratio = (float)width / (float)height;
		RHI::GetD3DContext()->RSSetViewports(1, &viewport);*/
		RHI::ResizeContext(width, height);
		//	RHI::GetD3DContext()->rs
	//	XM_PIDIV2
		//m_proj = XMMatrixPerspectiveFovLH(XM_PIDIV2*0.66667f, aspectratio, 0.1f, 1000.0f);
		m_proj = glm::perspectiveFovLH(XM_PIDIV2*0.66667f, (float)width, (float)height, 0.1f, 1000.0f);
		m_Cam->UpdateProjection((float)width / (float)height);
	}
}

void D3D11Window::DestroyRenderWindow()
{
	DestroyWindow(m_hwnd);
}

BOOL D3D11Window::MouseLBDown(int x, int y)
{
	m_input->MouseLBDown(x, y);
	return TRUE;
}

BOOL D3D11Window::MouseLBUp(int x, int y)
{
	m_input->MouseLBUp(x, y);
	return TRUE;
}

BOOL D3D11Window::MouseMove(int x, int y)
{
	m_input->MouseMove(x, y, 0.01f);
	return TRUE;
}

BOOL D3D11Window::KeyDown(WPARAM key)
{
	m_input->ProcessKeyDown(key);
	return 0;
}

BOOL D3D11Window::KeyUp(WPARAM )
{
	return 0;
}

BOOL D3D11Window::KeyHeld(WPARAM )
{
	return 0;
}

BOOL D3D11Window::MouseRBDown(int , int )
{
	return 0;
}
BOOL D3D11Window::MouseRBUp(int , int )
{
	return 0;
}

void D3D11Window::ProcessMenu(WORD )
{
}

#endif