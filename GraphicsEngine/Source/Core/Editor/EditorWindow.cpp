#include "EditorWindow.h"
#include "Resource.h"
#include <iomanip> 
#include <string>
#include <sstream>
#include "include/glm/gtc/type_ptr.hpp"
#include <ctime>
#include <time.h>
#include <memory>
#include "Core/BaseWindow.h"
#include "Core/Transform.h"
#include "Core/GameObject.h"
#include "Core/Input.h"
#include "Rendering/Core/Material.h"
#include "Rendering/Renderers/ForwardRenderer.h"
#include "Rendering/Renderers/DeferredRenderer.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "Rendering/Renderers/RenderSettings.h"
#include "Core/Engine.h"
#include "Physics/PhysicsEngine.h"

#include "UI/UIManager.h"
#include "RHI/RHI.h"
#include "EditorGizmos.h"
#include "UI/EditorUI/UIEditField.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Assets/AssetManager.h"
#include "EditorObjectSelector.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Core/Assets/SceneJSerialiser.h"
#include "Core/Utils/StringUtil.h"
#include "Core/Game.h"
#include "Editor/Editor_Camera.h"
#include "UI/UIManager.h"
#include "EditorCore.h"
EditorWindow* EditorWindow::instance;
EditorWindow::EditorWindow(bool Isdef) :BaseWindow()
{
	IsDeferredMode = Isdef;
	instance = this;
	mEditorCore = new EditorCore();
}

EditorWindow::~EditorWindow()
{
	//vectors will clean up after them selves
	//Clean up the renderable
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


EditorWindow::EditorWindow(HINSTANCE, int, int)
{
	instance = this;
}

void EditorWindow::PostInitWindow(int w, int h)
{
	Log::OutS  << "Loading Editor v0.01" << Log::OutS;
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

	if (RHI::IsD3D12())
	{
		CurrentScene->LoadExampleScene(Renderer, false);
		RefreshScene();
	}
	
}

void EditorWindow::EnterPlayMode()
{
	Log::OutS  << "Entering play mode" << Log::OutS;
	Engine::GetGame()->BeginPlay();
	IsPlayingScene = true;
	if (CurrentPlayScene != nullptr)
	{
		delete CurrentPlayScene;
	}
#if PLAYMODE_USE_SAVED
	SaveScene();
	CurrentPlayScene = new Scene();
	Saver->LoadScene(CurrentPlayScene, CurrentSceneSavePath);
	//CurrentScene->CopyScene(CurrentPlayScene);
	//CurrentPlayScene->LoadExampleScene(Renderer, false);
	Renderer->SetScene(CurrentPlayScene);
	CurrentPlayScene->StartScene();
#endif
	EditorCamera->SetEnabled(false);
	IsRunning = true;

}

void EditorWindow::ExitPlayMode()
{
	Log::OutS  << "Exiting play mode" << Log::OutS;
	Renderer->SetScene(CurrentScene);
	EditorCamera->SetEnabled(true);
	IsPlayingScene = false;
	IsRunning = false;
}

void EditorWindow::DestroyRenderWindow()
{
	BaseWindow::DestroyRenderWindow();
}

bool EditorWindow::MouseLBDown(int x, int y)
{
	BaseWindow::MouseLBDown(x, y);
	if (input != nullptr && UI != nullptr && !UI->IsUIBlocking())
	{
		mEditorCore->SetSelectedObject(selector->RayCastScene(x, y, EditorCamera->GetCamera(), *CurrentScene->GetObjects()));
	}
	return 0;
}

void EditorWindow::SetDeferredState(bool state)
{
	IsDeferredMode = state;
}

EditorCore * EditorWindow::GetEditorCore()
{
	if (instance != nullptr)
	{
		return instance->mEditorCore; 
	}
	return nullptr;
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
	if (IsPlayingScene)
	{
		PerfManager::StartTimer("Scene Update");
		CurrentPlayScene->UpdateScene(DeltaTime);
		PerfManager::EndTimer("Scene Update");
	}
	else
	{
		CurrentScene->EditorUpdateScene();
	}
	EditorCamera->Update(DeltaTime);

	if (mEditorCore->GetSelectedObject() != nullptr)
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
	if (input->GetVKey(VK_CONTROL))
	{
		if (input->GetKeyDown('S'))
		{
			SaveScene();
		}
	}
	if (input->GetKeyDown(VK_F2))
	{
		ExtendedPerformanceStats = !ExtendedPerformanceStats;
	}
	if (input->GetKeyDown(VK_F1))
	{
		ShowHud = !ShowHud;
	}
	
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

