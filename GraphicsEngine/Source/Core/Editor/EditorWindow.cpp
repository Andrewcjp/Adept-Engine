#include "EditorWindow.h"
#include "AI/Core/AISystem.h"
#include "AI/Core/Navigation/NavigationManager.h"
#include "Audio/AudioEngine.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Assets/SceneJSerialiser.h"
#include "Core/Game/Game.h"
#include "Core/Input/Input.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Utils/DebugDrawers.h"
#include "Core/Version.h"
#include "Editor_Camera.h"
#include "EditorCore.h"
#include "EditorGizmos.h"
#include "EditorObjectSelector.h"
#include "UI/UIManager.h"
#include "Rendering/VR/HMDManager.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Assets/Scene.h"
#include "Rendering/Core/ShadowRenderer.h"
#if WITH_EDITOR
EditorWindow* EditorWindow::instance = nullptr;
EditorWindow::EditorWindow() :BaseWindow()
{
	instance = this;
	mEditorCore = new EditorCore();
	EditorPlaySceneTempFile = AssetManager::GetDDCPath() + "EditorScene.tmp";
}

EditorWindow::~EditorWindow()
{
	//vectors will clean up after them selves
	//Clean up the Mesh
	SafeDelete(CurrentPlayScene);
	SafeDelete(CurrentScene);
	instance = nullptr;
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
#define TEST_SERIAL 0
void EditorWindow::PostInitWindow(int w, int h)
{
	ShowHud = true;
	ShowText = true;
	//SetFrameRateLimit(20);
	Log::OutS << "Loading Editor" << Log::OutS;
	CurrentScene = new Scene(true);
	EditorCamera = new Editor_Camera();

	SceneRenderer::Get()->SetEditorCamera(EditorCamera);
	if (UI != nullptr)
	{
		UI->InitGameobjectList(CurrentScene->GetObjects());
	}
	Log::LogMessage("Scene initialized");
	gizmos = new EditorGizmos();
	selector = new EditorObjectSelector();
	selector->init();

	CurrentScene->LoadExampleScene();
	SceneRenderer::Get()->SetScene(CurrentScene);
	RefreshScene();

#if TEST_SERIAL
	std::string TestFilePath = AssetManager::GetContentPath() + "Test\\Test.Scene";
	Saver->SaveScene(CurrentScene, TestFilePath);
	delete CurrentScene;
	CurrentScene = new Scene();
	Saver->LoadScene(CurrentScene, TestFilePath);
	Renderer->SetScene(CurrentScene);
	RefreshScene();
#endif

	AISystem::Get()->GenerateMesh();
}

EditorWindow* EditorWindow::GetInstance()
{
	return instance;
}

void EditorWindow::EnterPlayMode()
{
	if (IsPlayingScene)
	{
		return;
	}

	Log::OutS << "Entering play mode" << Log::OutS;
	const std::string PlayStatTimer = "Scene Play";
	PerfManager::Get()->StartSingleActionTimer(PlayStatTimer);
	Engine::GetGame()->BeginPlay();
	mEditorCore->SetSelectedObject(nullptr);
	SafeDelete(CurrentPlayScene);
#if PLAYMODE_USE_SAVED
	Saver->SaveScene(CurrentScene, EditorPlaySceneTempFile);
	CurrentPlayScene = new Scene();
	IsRunning = true;
	IsPlayingScene = true;
	ShouldTickScene = true;
	Saver->LoadScene(CurrentPlayScene, EditorPlaySceneTempFile);
	SceneRenderer::Get()->SetScene(CurrentPlayScene);
	CurrentPlayScene->StartScene();
#endif
	EditorCamera->SetEnabled(false);
	if (StartSimulate)
	{
		Eject();
	}
	PerfManager::Get()->EndSingleActionTimer(PlayStatTimer);
	PerfManager::Get()->LogSingleActionTimer(PlayStatTimer);
	PerfManager::Get()->FlushSingleActionTimer(PlayStatTimer);
}

void EditorWindow::ExitPlayMode()
{
	if (!IsPlayingScene)
	{
		return;
	}
	AudioEngine::StopAll();
	mEditorCore->SetSelectedObject(nullptr);
	Log::OutS << "Exiting play mode" << Log::OutS;
	SceneRenderer::Get()->SetScene(CurrentScene);
	CurrentPlayScene->EndScene();
	EditorCamera->SetEnabled(true);
	SafeDelete(CurrentPlayScene);
	IsPlayingScene = false;
	IsRunning = false;
	ShouldTickScene = false;
	StartSimulate = false;
}

void EditorWindow::DestroyRenderWindow()
{
	ExitPlayMode();
	BaseWindow::DestroyRenderWindow();
}

bool EditorWindow::MouseLBDown(int x, int y)
{
	BaseWindow::MouseLBDown(x, y);

	return 0;
}

EditorCore * EditorWindow::GetEditorCore()
{
	if (instance != nullptr)
	{
		return instance->mEditorCore;
	}
	return nullptr;
}

bool EditorWindow::UseSmallerViewPort()
{
#if EDITORUI
	return false;
#else
	return false;
#endif
}

void EditorWindow::PrePhysicsUpdate()
{}

void EditorWindow::DuringPhysicsUpdate()
{}

void EditorWindow::FixedUpdate()
{
	if (IsSceneRunning())
	{
		CurrentPlayScene->FixedUpdateScene(TickRate);
	}
}

bool EditorWindow::IsSceneRunning()
{
	return IsPlayingScene && !PauseState;
}

void EditorWindow::Update()
{
	if (IsSceneRunning() && CurrentPlayScene != nullptr)
	{
		PerfManager::StartTimer("Scene Update");
		CurrentPlayScene->UpdateScene(DeltaTime);
		PerfManager::EndTimer("Scene Update");
		if (IsEditorEjected())
		{
			CurrentPlayScene->EditorUpdateScene();
		}
	}
	else if (!IsScenePaused())
	{
		CurrentScene->EditorUpdateScene();
		AISystem::Get()->EditorTick();
	}
	if (!IsPlayingScene || IsEditorEjected())
	{
		EditorCamera->Update(DeltaTime);
	}
	if (mEditorCore->GetSelectedObject() != nullptr && !IsPlayingScene)
	{
		gizmos->Update(0);
		gizmos->SetTarget(mEditorCore->GetSelectedObject());
		gizmos->RenderGizmos(LineDrawer);
		if (UI != nullptr && UI->GetInspector() != nullptr)
		{
			UI->GetInspector()->SetSelectedObject(mEditorCore->GetSelectedObject());
		}
	}
	else
	{
#if !BASIC_RENDER_ONLY
		if (UI->GetInspector() != nullptr)
		{
			UI->GetInspector()->SetSelectedObject(nullptr);
		}
#endif
	}
	if (!Input::GetMouseButtonDown(1))
	{
		if (Input::GetKeyDown('W'))
		{
			gizmos->SwitchMode(CurrentGizmoMode::Translate);
		}
		if (Input::GetKeyDown('E'))
		{
			gizmos->SwitchMode(CurrentGizmoMode::Rotate);
		}
		if (Input::GetKeyDown('R'))
		{
			gizmos->SwitchMode(CurrentGizmoMode::Scale);
		}
	}
	if (Input::GetKeyDown('T'))
	{
		int currentmode = RHI::GetRenderSettings()->GetDebugRenderMode();
		currentmode++;
		currentmode = currentmode % (ERenderDebugOutput::Limit);
		RHI::GetRenderSettings()->SetDebugRenderMode((ERenderDebugOutput::Type)currentmode);
	}
	if (Input::GetKeyDown(VK_ESCAPE))
	{
		ExitPlayMode();
	}
	if (Input::GetKeyDown('P'))
	{
		ShadowRenderer::InvalidateAllBakedShadows();
	}
	if (Input::GetVKey(VK_CONTROL))
	{
		if (Input::GetKeyDown('S'))
		{
			SaveScene();
		}
	}
	if (Input::GetVKey(VK_CONTROL) && Input::GetKeyDown('P'))
	{
		EnterPlayMode();
	}
	if (Input::GetVKey(VK_CONTROL) && Input::GetKeyDown('O'))
	{
		StartSimulate = true;
		EnterPlayMode();
	}
	if (Input::GetKeyDown(VK_F8))
	{
		ShowHud = !ShowHud;
	}
	if (Input::GetKeyDown(VK_F4))
	{
		Eject();
	}
	if (Input::GetMouseButtonDown(0) && UI != nullptr && !UI->IsUIBlocking() && IsEditorEjected())
	{
		//mEditorCore->SetSelectedObject(selector->RayCastScene(Input::GetMousePos().x, Input::GetMousePos().y, EditorCamera->GetCamera(), *CurrentScene->GetObjects()));
	}
	if (RHI::SupportVR())
	{
		if (Input::GetKeyDown('L'))
		{
			RHI::GetVrSettings()->MirrorMode = EVRMirrorMode::Right;
		}
		if (Input::GetKeyDown('K'))
		{
			RHI::GetVrSettings()->MirrorMode = EVRMirrorMode::Left;
		}
		if (Input::GetKeyDown('B'))
		{
			RHI::GetVrSettings()->MirrorMode = EVRMirrorMode::Both;
		}
		if (Input::GetKeyDown(219))
		{
			RHI::GetVrSettings()->EyeDistance -= 0.1f;
		}
		if (Input::GetKeyDown(221))
		{
			RHI::GetVrSettings()->EyeDistance += 0.1f;
		}
	}
}
IntRect EditorWindow::GetViewPortRect()
{
	return UI->GetEditorRect();
}
void EditorWindow::Eject()
{
	if (IsPlayingScene)
	{
		EditorCamera->SetEnabled(!EditorCamera->GetEnabled());
		Input::LockCursor(false);
		Input::SetCursorVisible(true);
	}
}

bool EditorWindow::IsEditorEjected()
{
	return EditorCamera->GetEnabled();
}

bool EditorWindow::IsInPlayMode()
{
	return IsPlayingScene;
}

void EditorWindow::SaveScene()
{
	if (CurrentSceneSavePath.length() == 0)
	{
		std::string Startdir = Engine::GetExecutionDir();
		Startdir.append("\\asset\\scene\\");
#ifdef PLATFORM_WINDOWS
		using namespace std::string_literals;
		std::string Output;
		if (PlatformApplication::DisplaySaveFileDialog(Startdir, "Scene Files\0*.scene\0"s, ".scene", Output))
		{
			CurrentSceneSavePath = Output;
			Saver->SaveScene(CurrentScene, CurrentSceneSavePath);
		}
#endif
	}
	else
	{
		Saver->SaveScene(CurrentScene, CurrentSceneSavePath);
	}
}

void EditorWindow::LoadScene()
{
	mEditorCore->SetSelectedObject(nullptr);
	std::string Output;
	std::string Startdir = Engine::GetExecutionDir();
	Startdir.append("\\asset\\scene\\");
#ifdef PLATFORM_WINDOWS
	using namespace std::string_literals;
	if (PlatformApplication::DisplayOpenFileDialog(Startdir, "Scene Files\0*.scene\0"s, Output))
#endif
	{
		CurrentSceneSavePath = Output;
		SceneRenderer::Get()->SetScene(nullptr);
		//Renderer->SetScene(nullptr);
		delete CurrentScene;
		CurrentScene = new Scene();
		Saver->LoadScene(CurrentScene, Output);
		//Renderer->SetScene(CurrentScene);
		SceneRenderer::Get()->SetScene(CurrentScene);
		RefreshScene();
		UI->AlertBox("Scene Loaded");
	}
}

void EditorWindow::RefreshScene()
{
#if BASIC_RENDER_ONLY
	return;
#endif
	selector->LinkPhysxBodysToGameObjects(CurrentScene->GetObjects());
	UI->UpdateGameObjectList(CurrentScene->GetObjects());
	UI->RefreshGameObjectList();
}

void EditorWindow::WindowUI()
{

}

#endif