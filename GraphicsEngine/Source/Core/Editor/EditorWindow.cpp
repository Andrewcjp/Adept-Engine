#include "EditorWindow.h"
#include "Core/Input/Input.h"
#include "Rendering/Renderers/ForwardRenderer.h"
#include "Core/Version.h"
#include "UI/UIManager.h"
#include "EditorGizmos.h"
#include "Core/Assets/AssetManager.h"
#include "EditorObjectSelector.h"
#include "Core/Assets/SceneJSerialiser.h"
#include "Core/Game/Game.h"
#include "Editor/Editor_Camera.h"
#include "EditorCore.h"
#include "Core/Platform/PlatformCore.h"
#include "Audio/AudioEngine.h"
#include "AI/Core/AISystem.h"
#include "AI/Core/NavigationMesh.h"
#include "Core/Utils/DebugDrawers.h"
#if WITH_EDITOR
EditorWindow* EditorWindow::instance;
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
	delete CurrentScene;
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
	FrameRateLimit = 60;
	Log::OutS << "Loading Editor v" << EDITOR_VERSION << Log::OutS;
	CurrentScene = new Scene(true);
	EditorCamera = new Editor_Camera();
	Renderer->SetEditorCamera(EditorCamera);

	if (UI != nullptr)
	{
		UI->InitGameobjectList(CurrentScene->GetObjects());
	}
	fprintf(stdout, "Scene initalised\n");
	gizmos = new EditorGizmos();
	selector = new EditorObjectSelector();
	selector->init();

	CurrentScene->LoadExampleScene(Renderer, false);
	Renderer->SetScene(CurrentScene);
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
	//AISystem::Get()->mesh->GenTestMesh();
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
	Saver->LoadScene(CurrentPlayScene, EditorPlaySceneTempFile);
	Renderer->SetScene(CurrentPlayScene);
	CurrentPlayScene->StartScene();
#endif
	EditorCamera->SetEnabled(false);
	IsRunning = true;
	IsPlayingScene = true;
	ShouldTickScene = true;
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
	Renderer->SetScene(CurrentScene);
	CurrentScene->EndScene();
	EditorCamera->SetEnabled(true);
	IsPlayingScene = false;
	IsRunning = false;
	ShouldTickScene = false;
	SafeDelete(CurrentPlayScene);
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
	return ShowHud;
}

void EditorWindow::PrePhysicsUpdate()
{}

void EditorWindow::DuringPhysicsUpdate()
{}

void EditorWindow::FixedUpdate()
{
	if (IsPlayingScene)
	{
		CurrentPlayScene->FixedUpdateScene(TickRate);
	}
}

void EditorWindow::Update()
{
	if (IsPlayingScene && CurrentPlayScene != nullptr)
	{
		PerfManager::StartTimer("Scene Update");
		CurrentPlayScene->UpdateScene(DeltaTime);
		PerfManager::EndTimer("Scene Update");
	}
	else
	{
		CurrentScene->EditorUpdateScene();
	}

	DebugDrawers::DrawDebugCapsule(glm::vec3(0, 10, 0), 1, 2, glm::quat(glm::radians(glm::vec3(90, 0, 0))), glm::vec3(1,0,0));
	AISystem::Get()->mesh->DrawNavMeshLines(LineDrawer);
	if (!IsPlayingScene || IsEditorEjected())
	{
		EditorCamera->Update(DeltaTime);
	}
	if (mEditorCore->GetSelectedObject() != nullptr && !IsPlayingScene)
	{
		gizmos->Update(0);
		gizmos->SetTarget(mEditorCore->GetSelectedObject());
		gizmos->RenderGizmos(LineDrawer);
		if (UI != nullptr)
		{
			UI->GetInspector()->SetSelectedObject(mEditorCore->GetSelectedObject());
		}
	}
	else
	{
		UI->GetInspector()->SetSelectedObject(nullptr);
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
	if (Input::GetKeyDown(VK_ESCAPE))
	{
		ExitPlayMode();
	}
	if (Input::GetVKey(VK_CONTROL))
	{
		if (Input::GetKeyDown('S'))
		{
			SaveScene();
		}
	}

	if (Input::GetKeyDown(VK_F1))
	{
		ShowHud = !ShowHud;
	}
	if (Input::GetKeyDown(VK_F4))
	{
		Eject();
	}
	if (Input::GetMouseButtonDown(0) && UI != nullptr && !UI->IsUIBlocking() && IsEditorEjected())
	{
		mEditorCore->SetSelectedObject(selector->RayCastScene(Input::GetMousePos().x, Input::GetMousePos().y, EditorCamera->GetCamera(), *CurrentScene->GetObjects()));
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

void EditorWindow::SaveScene()
{
	if (CurrentSceneSavePath.length() == 0)
	{
		std::string Startdir = Engine::GetExecutionDir();
		Startdir.append("\\asset\\scene\\");
		using namespace std::string_literals;
		std::string Output;
		if (PlatformApplication::DisplaySaveFileDialog(Startdir, "Scene Files\0*.scene\0"s, ".scene", Output))
		{
			CurrentSceneSavePath = Output;
			Saver->SaveScene(CurrentScene, CurrentSceneSavePath);
		}
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
	using namespace std::string_literals;
	if (PlatformApplication::DisplayOpenFileDialog(Startdir, "Scene Files\0*.scene\0"s, Output))
	{
		CurrentSceneSavePath = Output;
		Renderer->SetScene(nullptr);
		delete CurrentScene;
		CurrentScene = new Scene();
		Saver->LoadScene(CurrentScene, Output);
		Renderer->SetScene(CurrentScene);
		RefreshScene();
		UI->AlertBox("Scene Loaded");
	}
}

void EditorWindow::RefreshScene()
{
	selector->LinkPhysxBodysToGameObjects(*CurrentScene->GetObjects());
	UI->UpdateGameObjectList(CurrentScene->GetObjects());
	UI->RefreshGameObjectList();
}

void EditorWindow::ProcessMenu(unsigned short command)
{
	switch (command)
	{
	case 4://add gameobject
		CurrentScene->AddGameobjectToScene(new GameObject("New GameObject"));
		UI->UpdateGameObjectList(CurrentScene->GetObjects());
		UI->RefreshGameObjectList();
		break;
	case 5://Save Scene
		SaveScene();
		break;
	case 6://Load Scene
		LoadScene();
		break;
	case 10://debug load example
		CurrentScene->LoadExampleScene(Renderer, false);
		Renderer->SetScene(CurrentScene);
		RefreshScene();
		CurrentSceneSavePath.clear();
		break;
	case 11:
		Engine::RunCook();
		break;
	}
}

void EditorWindow::WindowUI()
{

}

#endif