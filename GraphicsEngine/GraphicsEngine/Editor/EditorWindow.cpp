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
#include "../Core/Assets/SceneJSerialiser.h"
#include "../Core/Assets/AssetManager.h"
#include "../Core/Utils/StringUtil.h"
#include "../Core/Assets/SceneSerialiser.h"
#include "../Core/Utils/WindowsHelper.h"
#include "../Core/Game.h"
#include "../Editor/Editor_Camera.h"

UIWidget* EditorWindow::CurrentContext;
EditorWindow* EditorWindow::instance;
EditorWindow::EditorWindow(bool Isdef)
{
	IsDeferredMode = Isdef;
	instance = this;
}

EditorWindow::~EditorWindow()
{
	//vectors will clean up after them selves
	//Clean up the renderable
	delete CurrentScene;
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
		else if (command.find("vtest") != -1)
		{
		/*	instance->CurrentRenderSettings.CurrentAAMode = (instance->CurrentRenderSettings.CurrentAAMode == AAMode::FXAA) ? AAMode::NONE : AAMode::FXAA;
			instance->Renderer->SetRenderSettings(instance->CurrentRenderSettings);*/
			ForwardRenderer* r = (ForwardRenderer*)instance->Renderer;
			r->UseQuerry = r->UseQuerry ? false : true;
			return true;
		}
	}
	return false;
}

Scene * EditorWindow::GetCurrentScene()
{
	if (instance != nullptr)
	{
		if (instance->IsPlayingScene)
		{
			return instance->CurrentPlayScene;
		}
		return instance->CurrentScene;
	}
	return nullptr;
}

void EditorWindow::SetContext(UIWidget * target)
{
	CurrentContext = target;
}

float EditorWindow::GetDeltaTime()
{
	if (instance != nullptr)
	{
		return instance->deltatime;
	}
	return 0;
}

EditorWindow::EditorWindow(HINSTANCE hInstance, int width, int height)
{
	instance = this;
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
void ChangeDisplayMode(int width, int height)
{
	//int width = 1920;
	//int height = 1080;
	//HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_POPUP,
	//	0, 0, width, height, nullptr, nullptr, hInstance, nullptr);
	///*HWND hWnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
	//L"RenderWindow", L"OGLWindow", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
	//0, 0, 150, 150, NULL, NULL, hInstance, NULL);*/
	//ShowWindow(hWnd, 0);
	if (true)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		EnumDisplaySettings(NULL, 0, &dmScreenSettings);
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.
		LONG result = ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
		if (result != DISP_CHANGE_SUCCESSFUL)
		{
			__debugbreak();
		}
	}

}
bool EditorWindow::CreateRenderWindow(HINSTANCE hInstance, int width, int height, bool Fullscreen)
{

	if (Fullscreen)
	{
		ChangeDisplayMode(width, height);
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
	SceneFileLoader = new SceneSerialiser();
	CurrentScene = new Scene();
	Renderer->InitOGL();
	CurrentScene->LoadDefault(Renderer, IsDeferredMode);
	Renderer->SetScene(CurrentScene);
	Renderer->Init();
	EditorCamera = new Editor_Camera();
	Renderer->SetEditorCamera(EditorCamera);
	if (LoadText)
	{
		UI = std::make_unique<UIManager>(m_width, m_height);
		UI->InitGameobjectList(CurrentScene->GetObjects());
	}
	input = std::make_unique<Input>(nullptr, nullptr, m_hwnd, this);
	input->main = dynamic_cast<Shader_Main*>(Renderer->GetMainShader());
	input->Renderer = Renderer;
	input->Filters = Renderer->GetFilterShader();
	input->Cursor = CopyCursor(LoadCursor(NULL, IDC_ARROW));
	input->Cursor = SetCursor(input->Cursor);
	gizmos = new EditorGizmos();
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
	PerfManager::StartPerfManager();
	Saver = new SceneJSerialiser();
	Saver->SaveScene(CurrentScene);
	return TRUE;
}

void EditorWindow::EnterPlayMode()
{
	std::cout << "Entering play mode" << std::endl;
	Engine::GetGame()->BeginPlay();
	IsPlayingScene = true;
	if (CurrentPlayScene != nullptr)
	{
		delete CurrentPlayScene;
	}
	CurrentPlayScene = new Scene();
	//CurrentScene->CopyScene(CurrentPlayScene);
	CurrentPlayScene->LoadDefault(Renderer, false);
	Renderer->SetScene(CurrentPlayScene);
	CurrentPlayScene->StartScene();
	EditorCamera->SetEnabled(false);
}
void EditorWindow::ExitPlayMode()
{
	std::cout << "Exiting play mode" << std::endl;
	Renderer->SetScene(CurrentScene);
	EditorCamera->SetEnabled(true);
}
int EditorWindow::GetWidth()
{
	if (instance != nullptr)
	{
		return instance->m_width;
	}
	return 0;
}
int EditorWindow::GetHeight()
{
	if (instance != nullptr)
	{
		return instance->m_height;
	}
	return 0;
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
		stream << "GPU :" << PerfManager::GetGPUTime() << "ms ";
		stream << "CPU " << std::setprecision(4) << PerfManager::GetCPUTime() << "ms ";
		//	stream << "Frame :" << PerfManager::GetDeltaTime()*1000.0f << "ms ";
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
inline void EditorWindow::SetDeferredState(bool state)
{
	IsDeferredMode = state;
}
void EditorWindow::Render()
{
	if (PerfManager::Instance != nullptr)
	{
		deltatime = PerfManager::GetDeltaTime();
		PerfManager::Instance->StartCPUTimer();
		PerfManager::Instance->StartFrameTimer();
	}
	accumrendertime += deltatime;
	input->ProcessInput((deltatime));
	input->ProcessQue();
	//editor Functions
	Update();
	EditorCamera->Update(deltatime);

	//float targettime = (1.0f / 61.0f);
	//if ((accumrendertime) < targettime)
	//{
	//	//return;
	//}
	//accumrendertime = 0;
	if (IsPlayingScene)
	{
		//PerfManager::StartTimer("Scene Update");
		CurrentPlayScene->UpdateScene(deltatime);
		//PerfManager::EndTimer("Scene Update");
	}
	else
	{
		CurrentScene->EditorUpdateScene();
	}

	//lock the simulation rate to 60hz
	//this prevents physx being framerate depenent.
	float TickRate = 1.0f / 60.0f;
	if (accumilatePhysxdeltatime > TickRate)
	{
		accumilatePhysxdeltatime = 0;
		if (IsPlayingScene)
		{
			PerfManager::StartTimer("FTick");
			Engine::PhysEngine->stepPhysics(false, TickRate);
			CurrentPlayScene->FixedUpdateScene(TickRate);
			Renderer->FixedUpdatePhysx(TickRate);
			PerfManager::EndTimer("FTick");
		}
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
	if (PerfManager::Instance != nullptr)
	{
		PerfManager::Instance->StartGPUTimer();
	}
	Renderer->Render();
	dLineDrawer->GenerateLines();
	if (Renderer->GetMainCam() != nullptr)
	{
		dLineDrawer->RenderLines(Renderer->GetMainCam()->GetViewProjection());
	}
	Renderer->FinaliseRender();
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
	if (PerfManager::Instance != nullptr)
	{
		PerfManager::Instance->EndGPUTimer();
	}

	accumilatePhysxdeltatime += deltatime;
	framecount++;
	//	CPUTime = (float)((get_nanos() - FinalTime) / 1e6f);//in ms
	if (PerfManager::Instance != nullptr)
	{
		PerfManager::Instance->EndCPUTimer();
	}
#if !NO_GEN_CONTEXT
	RHI::RHISwapBuffers();
	//	SwapBuffers(m_hdc);
#endif

	if (PerfManager::Instance != nullptr)
	{
		PerfManager::Instance->SampleNVCounters();
		PerfManager::Instance->EndFrameTimer();
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

inline RenderEngine * EditorWindow::GetCurrentRenderer()
{
	return Renderer;
}

BOOL EditorWindow::MouseLBDown(int x, int y)
{
	if (UI != nullptr)
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
	if (UI != nullptr)
	{
		if (!UI->IsUIBlocking())
		{
			input->MouseLBDown(x, y);
		}
	}
	else
	{
		input->MouseLBDown(x, y);
	}

	return 0;
}

BOOL EditorWindow::MouseRBUp(int x, int y)
{
	if (UI)
	{
		if (!UI->IsUIBlocking())
		{
			input->MouseLBUp(x, y);
		}
	}
	else
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

void EditorWindow::ProcessMenu(WORD command)
{

	switch (command)
	{
	case 4://add gameobject

		break;
	case 5://Save Scene
		Saver->SaveScene(CurrentScene);
		break;
	case 6://Load Scene
		Renderer->SetScene(nullptr);
		delete CurrentScene;
		CurrentScene = new Scene();
		Saver->LoadScene(CurrentScene);
		Renderer->SetScene(CurrentScene);
		UI->UpdateGameObjectList(CurrentScene->GetObjects());
		UI->RefreshGameObjectList();
		break;
	default:
		break;
	}

}

