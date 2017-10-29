#include "EditorWindow.h"
#include "Resource.h"
#include <iomanip> 
#include <string>
#include <sstream>
#include "include/glm/gtc/type_ptr.hpp"
#include "Core/Assets/ImageLoader.h"
#include <gl/glu.h>
#include "Core/Engine.h"
#include "../Physics/PhysicsEngine.h"
#include "../EngineGlobals.h"
#include "UI/UIManager.h"
#include "RHI/RHI.h"
#include "EditorGizmos.h"
#include "UI/UIEditField.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Assets/AssetManager.h"
#include "EditorObjectSelector.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "../Core/Components/MeshRendererComponent.h"
#include "../Core/Assets/AssetManager.h"
#include "../Core/Utils/StringUtil.h"

UIWidget* EditorWindow::CurrentContext;
EditorWindow* EditorWindow::instance;
EditorWindow::EditorWindow(bool Isdef)
{
	IsDeferredMode = Isdef;
	m_euler[0] = m_euler[1] = m_euler[2] = 0.0f;
	instance = this;
}

EditorWindow::~EditorWindow()
{
	//vectors will clean up after them selves
	//Clean up the renderable
	delete CurrentScene;
}

float EditorWindow::GetFrameTime()
{
	if (instance != nullptr)
	{
		return (float)instance->avgtime * 1000.0f;
	}
	return 0;
}

float EditorWindow::GetCPUTime()
{
	if (instance != nullptr)
	{
		return instance->CPUTime;
	}
	return 0;
}

bool EditorWindow::ProcessDebugCommand(std::string command)
{
	if (instance != nullptr)
	{
		if (command.find("stats") != -1)
		{
			instance->ExtendedPerformanceStats = instance->ExtendedPerformanceStats ? false : true;
			return true;
		}
		else if (command.find("showparticles") != -1)
		{
			ForwardRenderer* r = (ForwardRenderer*)instance->Renderer;
			r->RenderParticles = r->RenderParticles ? false : true;
			return true;
		}
		else if (command.find("showgrass") != -1)
		{
			ForwardRenderer* r = (ForwardRenderer*)instance->Renderer;
			r->RenderGrass = r->RenderGrass ? false : true;
			return true;
		}
		else if (command.find("renderscale") != -1)
		{
			StringUtils::RemoveChar(command, ">renderscale ");
			instance->CurrentRenderSettings.RenderScale = glm::clamp(stof(command), 0.1f, 5.0f);
			instance->Renderer->SetRenderSettings(instance->CurrentRenderSettings);
			instance->Resize(instance->m_width, instance->m_height);
			return true;
		}
		else if (command.find("fxaa") != -1)
		{
			instance->CurrentRenderSettings.CurrentAAMode = (instance->CurrentRenderSettings.CurrentAAMode == AAMode::FXAA) ? AAMode::NONE : AAMode::FXAA;
			instance->Renderer->SetRenderSettings(instance->CurrentRenderSettings);
			return true;
		}
	}
	return false;
}

void EditorWindow::SetContext(UIWidget * target)
{
	CurrentContext = target;
}

EditorWindow::EditorWindow(HINSTANCE hInstance, int width, int height)
{
	//InitWindow(hInstance, width, height);
	instance = this;
	m_euler[0] = m_euler[1] = m_euler[2] = 0.0f;
}

void EditorWindow::DestroyRenderWindow()
{
	Renderer->DestoryRenderWindow();
	delete Renderer;
	UI.reset();
	input.reset();
	RHI::DestoryContext(m_hwnd);
	DestroyWindow(m_hwnd);

	m_hwnd = NULL;
	m_hdc = NULL;
}
//
//int DisplayConfirmSaveAsMessageBox()
//{
//	int msgboxID = MessageBox(
//		NULL,
//		L"temp.txt already exists.\nDo you want to replace it?",
//		L"Confirm Save As",
//		MB_ICONEXCLAMATION | MB_YESNO
//	);
//
//	if (msgboxID == IDYES)
//	{
//
//	}
//
//	return msgboxID;
//}

void EditorWindow::SwitchFullScreen(HINSTANCE hInstance)
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
//HWND CreateFullscreenWindow(HWND hwnd, HINSTANCE hInstance)
//{
//	HMONITOR hmon = MonitorFromWindow(hwnd,
//		MONITOR_DEFAULTTONEAREST);
//	MONITORINFO mi = { sizeof(mi) };
//	if (!GetMonitorInfo(hmon, &mi)) return NULL;
//	return CreateWindow(TEXT("static"),
//		TEXT("something interesting might go here"),
//		WS_POPUP | WS_VISIBLE,
//		mi.rcMonitor.left,
//		mi.rcMonitor.top,
//		mi.rcMonitor.right - mi.rcMonitor.left,
//		mi.rcMonitor.bottom - mi.rcMonitor.top,
//		hwnd, NULL, hInstance, 0);
//}
bool EditorWindow::CreateRenderWindow(HINSTANCE hInstance, int width, int height)
{

	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		L"RenderWindow", L"OGLWindow", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, width, height, NULL, NULL, hInstance, NULL);
	if (IsFullscreen)
	{
		//m_hwnd = CreateFullscreenWindow(m_hwnd, hInstance);
	}
	m_hInstance = hInstance;
	RHI::InialiseContext(m_hwnd, width, height);
	InitWindow(m_hglrc, m_hwnd, m_hdc, width, height);
	return true;
}
BOOL EditorWindow::InitWindow(HGLRC hglrc, HWND hwnd, HDC hdc, int width, int height)
{
	m_hdc = hdc;
	m_hwnd = hwnd;
	m_hglrc = hglrc;
	PerfManager::StartPerfManager();
	glewInit();
#if !NO_GEN_CONTEXT
	if (RHI::GetType() == ERenderSystemType::RenderSystemOGL)
	{
		Engine::setVSync(false);
	}
#endif
	if (RHI::GetType() == RenderSystemD3D11)
	{
		ShowHud = false;
		LoadText = false;
	}
	std::cout << "Scene Load started" << std::endl;
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
	CurrentScene = new Scene();
	CurrentScene->LoadDefault();
	Renderer->SetScene(CurrentScene);
	Renderer->InitOGL();

	Light* l = new Light(glm::vec3(0, 10, 20), 150, Light::Point, glm::vec3(1, 1, 1));

	//l->SetShadow(false);
	Renderer->AddLight(l);
	l = new Light(glm::vec3(30, 75, 50), 0.5, Light::Directional, glm::vec3(1, 1, 1));
	l->SetDirection(l->GetPosition());
	l->SetShadow(false);
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
	//Lights.push_back(new Light(glm::vec3(-15, 10, 0), 150, glm::vec3(0, 1, 0), Light::Spot, false));
	Renderer->Init();

	GameObject* go = new GameObject("House");

	Material* newmat = new Material(RHI::CreateTexture("../asset/texture/house_diffuse.tga", true));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("../asset/models/house.obj", Renderer->GetMainShader()->GetShaderProgram()), newmat));

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
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("../asset/models/terrainmk2.obj", Renderer->GetMainShader()->GetShaderProgram()), mat));

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

	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("../asset/models/Plane.obj", Renderer->GetMainShader()->GetShaderProgram()), mat));
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
		go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("../asset/models/Plane.obj", Renderer->GetMainShader()->GetShaderProgram()), mat));
		//		go->SetMesh(RHI::CreateMesh("../asset/models/Plane.obj", Renderer->GetMainShader()->GetShaderProgram()));
				//go->GetMat()->SetShadow(false);
		go->GetTransform()->SetPos(glm::vec3(-10, 1, -6));
		go->GetTransform()->SetEulerRot(glm::vec3(90, -90, 0));
		go->GetTransform()->SetScale(glm::vec3(0.1f));

		Renderer->AddGo(go);
	}


	go = new GameObject("Static Water");
	mat = new Material(RHI::CreateTexture("../asset/texture/Water fallback.jpg"));
	mat->NormalMap = RHI::CreateTexture("../asset/texture/IKT4l.jpg");

	//go->SetMaterial(mat);
//	go->SetMesh(RHI::CreateMesh("../asset/models/Plane.obj", Renderer->GetMainShader()->GetShaderProgram()));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("../asset/models/Plane.obj", Renderer->GetMainShader()->GetShaderProgram()), mat));
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

		//go->SetMaterial(mat);
		//go->SetMesh(RHI::CreateMesh("../asset/models/Plane.obj", Renderer->GetMainShader()->GetShaderProgram()));
		go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("../asset/models/Plane.obj", Renderer->GetMainShader()->GetShaderProgram()), mat));
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
	if (LoadText)
	{
		UI = std::make_unique<UIManager>(m_width, m_height);
		UI->InitGameobjectList(CurrentScene->GetObjects());
	}
	input = std::make_unique<Input>(Renderer->GetMainCam(), nullptr, m_hwnd, this);
	input->main = dynamic_cast<Shader_Main*>(Renderer->GetMainShader());
	input->Renderer = Renderer;
	input->Filters = Renderer->GetFilterShader();
	input->Cursor = CopyCursor(LoadCursor(NULL, IDC_ARROW));
	input->Cursor  = SetCursor(input->Cursor);
	gizmos = new EditorGizmos();
	//gizmos->SetTarget(Renderer->GetObjects()[2]);
	fprintf(stdout, "Scene initalised\n");

	startms = static_cast<float>(clock() / CLOCKS_PER_SEC);
	fpsnexttime = startms + 1;
	if (PerfManager::Instance != nullptr)
	{
		PerfManager::Instance->SampleNVCounters();
	}
	selector = new EditorObjectSelector();
	selector->init();
	selector->LinkPhysxBodysToGameObjects(Renderer->GetObjects());
	dLineDrawer = new DebugLineDrawer();
	CurrentScene->StartScene();
	glGenQueries(1, &query);
	return TRUE;
}

void EditorWindow::RenderText()
{
	if (!ShowText)
	{
		return;
	}
	std::stringstream stream;
	stream << (currenfps) << std::fixed << std::setprecision(1) << " " << (avgtime * 1000) << "ms ";
	if (ExtendedPerformanceStats)
	{
		stream << "CPU " << std::setprecision(4) << CPUTime << "ms " << std::setprecision(1);
		if (PerfManager::Instance != nullptr)
		{
			stream << PerfManager::Instance->GetAllTimers();
			stream << PerfManager::Instance->GetCounterData();
		}
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
//used for Updating Input for Editor Functions
void EditorWindow::Update()
{
	if (GetKeyState(VK_CONTROL) & 0x8000)
	{
		if (GetKeyState('S') & 0x8000)
		{
			//save the scene
		}
	}
}
void EditorWindow::Render()
{

	deltatime = (float)(get_nanos() - lasttime) / 1.0e9f;//in ms
	lasttime = get_nanos();
	FinalTime = get_nanos();
	accumrendertime += deltatime;
	input->ProcessInput((deltatime));
	input->ProcessQue();
	Update();
	float targettime = (1.0f / 61.0f);
	if ((accumrendertime) < targettime)
	{
		//return;
	}
	accumrendertime = 0;

	//	PerfManager::StartTimer("Scene Update");
	CurrentScene->UpdateScene(deltatime);
	//	PerfManager::EndTimer("Scene Update");

	//lock the simulation rate to 60hz
	//this prevents physx being framerate depenent.
	float TickRate = 1.0f / 60.0f;
	if (accumilatePhysxdeltatime > TickRate)
	{
		PerfManager::StartTimer("Tick");
		accumilatePhysxdeltatime = 0;
		Engine::PhysEngine->stepPhysics(false, TickRate);
		CurrentScene->FixedUpdateScene(TickRate);
		Renderer->FixedUpdatePhysx(TickRate);
		PerfManager::EndTimer("Tick");
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
	PerfManager::StartTimer("GPU");
	Renderer->Render();
	dLineDrawer->GenerateLines();
	dLineDrawer->RenderLines(Renderer->GetMainCam()->GetViewProjection());
	Renderer->FinaliseRender();
	PerfManager::EndTimer("GPU");
	if (input->Selectedobject != nullptr)
	{
		gizmos->SetTarget(input->Selectedobject);
		gizmos->RenderGizmos(dLineDrawer);
		UI->GetInspector()->SetSelectedObject(input->Selectedobject);
	}

	PerfManager::StartTimer("UI");
	if (UI != nullptr && ShowHud && LoadText)
	{
		UI->RenderWidgets();
	}
	if (LoadText)
	{
		RenderText();
	}
	PerfManager::EndTimer("UI");
	accumilatePhysxdeltatime += deltatime;
	framecount++;
	CPUTime = (float)((get_nanos() - FinalTime) / 1e6f);//in ms
#if !NO_GEN_CONTEXT
	RHI::RHISwapBuffers();
	//	SwapBuffers(m_hdc);
#endif
	if (PerfManager::Instance != nullptr)
	{
		PerfManager::Instance->SampleNVCounters();
	}
	if (Once)
	{
		std::cout << "Engine Loaded in " << fabs((get_nanos() - Engine::StartTime) / 1e6f) << "ms " << std::endl;
		Once = false;
	}
	input->Clear();//clear key states
}

void EditorWindow::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	//	glViewport(0, 0, width, height);
	if (UI != nullptr)
	{
		UI->UpdateSize(width, height);
	}
	if (Renderer != nullptr)
	{
		Renderer->Resize(width, height);
	}

	return;
}

BOOL EditorWindow::MouseLBDown(int x, int y)
{
	if (selector != nullptr && (!UI->IsUIBlocking()))
	{
		GameObject* target = selector->RayCastScene(x, y, Renderer->GetMainCam(), Renderer->GetObjects());
		if (target != nullptr)
		{
			std::cout << "target " << target->GetName() << std::endl;
			input->Selectedobject = target;
		}
	}
	if (UI != nullptr)
	{
		UI->MouseClick(x, y);
	}
	return TRUE;
}

BOOL EditorWindow::MouseLBUp(int x, int y)
{
	if (UI != nullptr)
	{
		UI->MouseClickUp(x, y);
	}
	return TRUE;
}

BOOL EditorWindow::MouseRBDown(int x, int y)
{
	if (!UI->IsUIBlocking())
	{
		input->MouseLBDown(x, y);
	}
	return 0;
}

BOOL EditorWindow::MouseRBUp(int x, int y)
{
	if (!UI->IsUIBlocking())
	{
		input->MouseLBUp(x, y);
	}

	return 0;
}

BOOL EditorWindow::MouseMove(int x, int y)
{
	input->MouseMove(x, y, deltatime);
	if (UI != nullptr)
	{
		UI->MouseMove(x, y);
	}
	return TRUE;
}

BOOL EditorWindow::KeyDown(WPARAM key)
{
	if (CurrentContext != nullptr)
	{
		CurrentContext->ProcessKeyDown(key);
	}
	else
	{
		input->ProcessKeyDown(key);
	}
	return TRUE;
}

