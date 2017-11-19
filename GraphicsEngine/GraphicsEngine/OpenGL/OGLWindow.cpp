#include "OGLWindow.h"
#include "Resource.h"
//#include "Matrix4x4.h"
//#include "Vector4.h"
#include <iomanip> 
#include <string>
#include <sstream>
#include "include/glm/gtc/type_ptr.hpp"
#include "Core/Assets/ImageLoader.h"
#include <gl/glu.h>
#include "Core/Engine.h"
#include "../Physics/PhysicsEngine.h"
#include "../EngineGlobals.h"
#include "UI\UIManager.h"
#include "RHI/RHI.h"
#include "Editor/EditorGizmos.h"
//#include "Utils.h"
OGLWindow::OGLWindow(bool Isdef)
{
	IsDeferredMode = Isdef;
	m_euler[0] = m_euler[1] = m_euler[2] = 0.0f;
}

OGLWindow::~OGLWindow()
{
	//vectors will clean up after them selves
	//Clean up the renderable
}

OGLWindow::OGLWindow(HINSTANCE hInstance, int width, int height)
{
}

void OGLWindow::DestroyRenderWindow()
{
	Renderer->DestoryRenderWindow();
	delete Renderer;
	RHI::DestoryContext(m_hwnd);

	DestroyWindow(m_hwnd);

	m_hwnd = NULL;
	m_hdc = NULL;
}

int DisplayConfirmSaveAsMessageBox()
{
	int msgboxID = MessageBox(
		NULL,
		L"temp.txt already exists.\nDo you want to replace it?",
		L"Confirm Save As",
		MB_ICONEXCLAMATION | MB_YESNO
	);

	if (msgboxID == IDYES)
	{

	}

	return msgboxID;
}

void OGLWindow::SwitchFullScreen(HINSTANCE hInstance)
{
	if (IsFullscreen)
	{
		IsFullscreen = false;
		m_hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
			L"RenderWindow", L"OGLWindow", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			0, 0, m_width, m_height, NULL, NULL, hInstance, NULL);
	}
	else
	{
		m_hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
			L"RenderWindow", L"OGLWindow", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			0, 0, m_width, m_height, NULL, NULL, hInstance, NULL);
	}
}
HWND CreateFullscreenWindow(HWND hwnd, HINSTANCE hInstance)
{
	HMONITOR hmon = MonitorFromWindow(hwnd,
		MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = { sizeof(mi) };
	if (!GetMonitorInfo(hmon, &mi)) return NULL;
	return CreateWindow(TEXT("static"),
		TEXT("something interesting might go here"),
		WS_POPUP | WS_VISIBLE,
		mi.rcMonitor.left,
		mi.rcMonitor.top,
		mi.rcMonitor.right - mi.rcMonitor.left,
		mi.rcMonitor.bottom - mi.rcMonitor.top,
		hwnd, NULL, hInstance, 0);
}
void OGLWindow::SetFullScreenState(bool state)
{
	if (state)
	{
		DEVMODE newSettings;
		// now fill the DEVMODE with standard settings, mainly monitor frequenzy
		EnumDisplaySettings(NULL, 0, &newSettings);
		// set desired screen size and resolution	
		newSettings.dmPelsWidth = m_width;
		newSettings.dmPelsHeight = m_height;
		newSettings.dmBitsPerPel = 16;
		//set those flags to let the next function know what we want to change
		newSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		// and apply the new settings
		if (ChangeDisplaySettings(&newSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			//__debugbreak();
		}
	}
	else
	{
		if (ChangeDisplaySettings(nullptr, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			//__debugbreak();
		}
	}
}
bool OGLWindow::CreateRenderWindow(HINSTANCE hInstance, int width, int height, bool Fullscreen)
{
	if (IsFullscreen)
	{
		//m_hwnd = CreateFullscreenWindow(m_hwnd, hInstance);
		m_hwnd = CreateWindowEx(NULL,
			L"RenderWindow", L"OGLWindow", WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			0, 0, width, height, NULL, NULL, hInstance, NULL);
	}
	else
	{
		m_hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
			L"RenderWindow", L"OGLWindow", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			0, 0, width, height, NULL, NULL, hInstance, NULL);
	}
	RHI::InialiseContext(m_hwnd, width, height);
	InitWindow(m_hglrc, m_hwnd, m_hdc, width, height);
	return true;
}
BOOL OGLWindow::InitWindow(HGLRC hglrc, HWND hwnd, HDC hdc, int width, int height)
{
	m_hdc = hdc;
	m_hwnd = hwnd;
	m_hglrc = hglrc;
	glewInit();
#if !NO_GEN_CONTEXT
	if (RHI::GetType() == ERenderSystemType::RenderSystemOGL)
	{
		Engine::setVSync(false);
	}
#endif

	std::cout << "Context Created" << std::endl;
	ImageLoader::StartLoader();
	m_width = width;
	m_height = height;
	if (IsDeferredMode)
	{
		Renderer = new DeferredRenderer(width, height);
	}
	else
	{
		Renderer = new ForwardRenderer(width, height);
	}

	Renderer->InitOGL();


	//shadowrender = new ShadowRenderer();
//	Renderer->AddLight(new Light(glm::vec3(0, 10, 0), 150, Light::Directional));
	Renderer->AddLight(new Light(glm::vec3(15, 10, 0), 200));
	Light* l = new Light(glm::vec3(0, 50, 30), 2500, Light::Point);
	//	l->SetDirection(l->GetPosition() - glm::vec3(0, 0, 0));
		//l->SetShadow(false);
	Renderer->AddLight(l);
	const int Girdsize = 5;
	glm::vec3 startpos = glm::vec3(0, 20, 50);
	/*const float spacing = 5;
	for (int x = 0; x < Girdsize; x++)
	{
		for (int y = 0; y < Girdsize; y++)
		{
			Light* l = new Light(glm::vec3(startpos.x + (x*spacing), 50, startpos.z + (y*spacing)), 2500, Light::Point);
			Renderer->AddLight(l);
		}
	}*/
	//	Lights.push_back(new Light(glm::vec3(-15, 10, 0), 150, glm::vec3(0, 1, 0), Light::p, false));
	Renderer->Init();

	GameObject* go = new GameObject("House");
	Material* newmat = new Material(RHI::CreateTexture("../asset/texture/house_diffuse.tga", true));
//	go->SetMaterial(newmat);
//	go->SetMesh(RHI::CreateMesh("../asset/models/house.obj", Renderer->GetMainShader()->GetShaderProgram()));
	//	m_mesh->position = glm::vec3(0, 0, -10);
	go->GetTransform()->SetPos(glm::vec3(7, 0, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	//	Objects.push_back(go);
	Renderer->AddGo(go);

	go = new GameObject("Terrain");


	Material* mat = new Material(RHI::CreateTexture("../asset/texture/grasshillalbedo.png"));
	//mat->NormalMap = new OGLTexture("../asset/texture/Normal.tga");
	//	mat->DisplacementMap = new OGLTexture("../asset/texture/bricks2_disp.jpg");
	//go->SetMaterial(mat);
	//go->SetMesh(RHI::CreateMesh("../asset/models/terrainmk2.obj", Renderer->GetMainShader()->GetShaderProgram()));
	//	go->GetMat()->SetShadow(false);
		//	m_mesh->position = glm::vec3(0, 0, -10);
	go->GetTransform()->SetPos(glm::vec3(0, 0, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(2));
	//slow
	Renderer->AddGo(go);



	go = new GameObject("Plane");

	mat = new Material(RHI::CreateTexture("../asset/texture/bricks2.jpg"));
	mat->NormalMap = RHI::CreateTexture("../asset/texture/bricks2_normal.jpg");
	mat->DisplacementMap = RHI::CreateTexture("../asset/texture/bricks2_disp.jpg");
	//go->SetMaterial(mat);
//	go->SetMesh(RHI::CreateMesh("../asset/models/Plane.obj", Renderer->GetMainShader()->GetShaderProgram()));
	//go->GetMat()->SetShadow(false);
	go->GetTransform()->SetPos(glm::vec3(-24, 2, -6));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(0.5));

	Renderer->AddGo(go);
	if (IsDeferredMode == false)
	{
		go = new GameObject("Fence");
		mat = new Material(RHI::CreateTexture("../asset/texture/fence.png"));
		mat->SetShadow(false);
//		go->SetMaterial(mat);
	//	go->SetMesh(RHI::CreateMesh("../asset/models/Plane.obj", Renderer->GetMainShader()->GetShaderProgram()));
		//go->GetMat()->SetShadow(false);
		go->GetTransform()->SetPos(glm::vec3(-10, 1, -6));
		go->GetTransform()->SetEulerRot(glm::vec3(90, -90, 0));
		go->GetTransform()->SetScale(glm::vec3(0.1f));

		Renderer->AddGo(go);
	}


	go = new GameObject("Static Water");
	mat = new Material(RHI::CreateTexture("../asset/texture/Water fallback.jpg"));
	mat->NormalMap = RHI::CreateTexture("../asset/texture/IKT4l.jpg");

//	go->SetMaterial(mat);
	//go->SetMesh(RHI::CreateMesh("../asset/models/Plane.obj", Renderer->GetMainShader()->GetShaderProgram()));
	//go->GetMat()->SetShadow(false);
	go->GetTransform()->SetPos(glm::vec3(-37, -2, -20));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(2));

	Renderer->AddGo(go);
	if (IsDeferredMode == false)
	{
		go = new GameObject("Water");

		mat = new Material(Renderer->GetReflectionBuffer()->GetRenderTexture());
		mat->NormalMap = RHI::CreateTexture("../asset/texture/IKT4l.jpg");

//		go->SetMaterial(mat);
	//	go->SetMesh(RHI::CreateMesh("../asset/models/Plane.obj", Renderer->GetMainShader()->GetShaderProgram()));
		//go->GetMat()->SetShadow(false);
		go->GetTransform()->SetPos(glm::vec3(-37, -1, -21));
		go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
		go->GetTransform()->SetScale(glm::vec3(2));
		go->SetReflection(true);
		Camera* c = new Camera(go->GetTransform()->GetPos(), 90.0f, static_cast<float>(m_width / m_height), 0.1f, 100.0f);
		c->Pitch(-90);
		c->SetUpAndForward(glm::vec3(0, 1.0, 0), glm::vec3(0, 0, 1.0));
		Renderer->SetReflectionCamera(c);
		Renderer->AddGo(go);
	}

	//	textrender = std::make_unique<TextRenderer>(m_width, m_height);
	UI = std::make_unique<UIManager>(m_width, m_height);
	//UI->InitGameobjectList(Renderer->GetObjects());
	input = std::make_unique<Input>(Renderer->GetMainCam(), nullptr, m_hwnd, this);
	input->main = dynamic_cast<Shader_Main*>(Renderer->GetMainShader());
	input->Renderer = Renderer;
	input->Filters = Renderer->GetFilterShader();
	gizmos = new EditorGizmos();
	gizmos->SetTarget(Renderer->GetObjects()[2]);
	fprintf(stdout, "Scene initalised\n");

	startms = static_cast<float>(clock() / CLOCKS_PER_SEC);
	fpsnexttime = startms + 1;


	return TRUE;
}

void OGLWindow::RenderText()
{
	std::stringstream stream;
	stream << (currenfps) << std::setprecision(3) << " " << (avgtime * 1000) << std::setprecision(3) << "ms";
	if (ExtendedPerformanceStats)
	{
		stream << " Sleep " << (sleeptimeMS / 1000) << std::setprecision(5) << " render " << (deltatime);
		//<< " render " << RenderTime * 1000<< std::setprecision(3) << " Shadow " << ShadowRendertime * 1000 << std::setprecision(3);
	}
	std::string data = stream.str();

	UI->RenderTextToScreen(1, data);
	stream.str("");
	std::string shape = "Box";
	if (input->GetShape() == 1)
	{
		shape = "Sphere";
	}
	stream << "Current force " << (input->GetForce()) << std::setprecision(6) << " Shape = " << shape;
	data = stream.str();
	UI->RenderTextToScreen(2, data);
	stream.str("");
	std::string go = "";
	std::string goname = "";
	if (input->Selectedobject != nullptr)
	{
		go = glm::to_string(input->Selectedobject->GetTransform()->GetPos());
		goname = input->Selectedobject->GetName();
	}
	stream << (input->currentObjectIndex) << " Obj " << goname << " Position " << go << std::setprecision(3);
	data = stream.str();
	UI->RenderTextToScreen(3, data);
}

void OGLWindow::Render()
{
	deltatime = (float)(get_nanos() - lasttime) / 1.0e9f;//in ms
	lasttime = get_nanos();

	input->ProcessInput((deltatime));
	input->ProcessQue();
	accumrendertime += deltatime;
	float targettime = (1.0f / 60.0f);//s
	if ((accumrendertime) < targettime)
	{
		//return;
	}
	accumrendertime = 0;
	//excute a linked fixed update.
	physxtime = get_nanos();
	//slow?
	Renderer->FixedUpdatePhysx(deltatime);
	physxtime = (get_nanos() - physxtime) / 1e9;
	//lock the physx simulation rate to 300HZ
	//this prevents physx being framerate depenent.
	float TickRate = 1.0f / 60.0f;
	if (accumilatePhysxdeltatime > TickRate)
	{
		accumilatePhysxdeltatime = 0;
		Engine::PhysEngine->stepPhysics(false, TickRate);
	}

	timesincestat = (((float)(clock()) / CLOCKS_PER_SEC));//in s
	if (timesincestat > fpsnexttime)
	{
		avgaccum++;
		currenfps = (double)((double)framecount / (fpsaccumtime));
		avgtime += (deltatime - avgtime) / avgaccum;
		fpsnexttime += fpsaccumtime;
		framecount = 0;
		if (avgaccum > 10)
		{
			avgaccum = 0;
		}
	}
	if (deltatime > 1)
	{
		std::cout << deltatime << " exceded one" << std::endl;
	}
	FinalTime = get_nanos();

	Renderer->Render();
	if (ShowHud)
	{
		//UI->RenderWidgets();
		RenderText();
	}
	//gizmos->RenderGizmos();
	RenderTime = (double)(get_nanos() - FinalTime) / 1e6;//in seconds
	accumilatePhysxdeltatime += deltatime;
	framecount++;
#if !NO_GEN_CONTEXT
	RHI::RHISwapBuffers();
	//	SwapBuffers(m_hdc);
#endif
}

void OGLWindow::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	//	glViewport(0, 0, width, height);
	if (UI != nullptr)
	{
		UI->UpdateSize(width, height);
		Renderer->Resize(width, height);
	}
	return;
}

BOOL OGLWindow::MouseLBDown(int x, int y)
{
	UI->MouseClick(x, y);

	return TRUE;
}

BOOL OGLWindow::MouseLBUp(int x, int y)
{
	UI->MouseClickUp(x, y);
	return TRUE;
}

BOOL OGLWindow::MouseRBDown(int x, int y)
{
	input->MouseLBDown(x, y);
	return 0;
}

BOOL OGLWindow::MouseRBUp(int x, int y)
{
	input->MouseLBUp(x, y);
	return 0;
}

BOOL OGLWindow::MouseMove(int x, int y)
{
	input->MouseMove(x, y, deltatime);
	UI->MouseMove(x, y);
	return TRUE;
}

BOOL OGLWindow::KeyDown(WPARAM key)
{
	input->ProcessKeyDown(key);
	return TRUE;
}



