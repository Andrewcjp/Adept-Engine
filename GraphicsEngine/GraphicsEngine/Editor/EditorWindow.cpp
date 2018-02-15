#include "EditorWindow.h"
#include "Resource.h"
#include <iomanip> 
#include <string>
#include <sstream>
#include "include/glm/gtc/type_ptr.hpp"
#include "Core/Assets/ImageLoader.h"
#include <ctime>
#include <time.h>
#include <GLEW\GL\glew.h>
#include <memory>
#include "Core/BaseWindow.h"
#include "Core/Transform.h"
#include "Core/GameObject.h"
#include "Core/Input.h"
#include "../Rendering/Core/Material.h"
#include "../Rendering/Renderers/ForwardRenderer.h"
#include "../Rendering/Renderers/DeferredRenderer.h"
#include "UI/TextRenderer.h"
#include "../Rendering/Renderers/RenderSettings.h"
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
#include "UI\UIManager.h"

EditorWindow* EditorWindow::instance;
EditorWindow::EditorWindow(bool Isdef) :BaseWindow()
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
	selector->LinkPhysxBodysToGameObjects(Renderer->GetObjects());
	Saver = new SceneJSerialiser();
	//Saver->SaveScene(CurrentScene);
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
	IsRunning = true;

}
void EditorWindow::ExitPlayMode()
{
	std::cout << "Exiting play mode" << std::endl;
	Renderer->SetScene(CurrentScene);
	EditorCamera->SetEnabled(true);
	IsPlayingScene = false;
	IsRunning = false;
}

BOOL EditorWindow::MouseLBDown(int x, int y)
{
	BaseWindow::MouseLBDown(x, y);
	if (input != nullptr && UI != nullptr && !UI->IsUIBlocking())
	{
		input->Selectedobject = selector->RayCastScene(x, y, EditorCamera->GetCamera(), *CurrentScene->GetObjects());
	}
	return 0;
}

void EditorWindow::SetDeferredState(bool state)
{
	IsDeferredMode = state;
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
	EditorCamera->Update(DeltaTime);

	if (input->Selectedobject != nullptr)
	{
		gizmos->Update(0);
		gizmos->SetTarget(input->Selectedobject);
		gizmos->RenderGizmos(dLineDrawer);
		if (UI != nullptr)
		{
			UI->GetInspector()->SetSelectedObject(input->Selectedobject);
		}
	}
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
	if (GetKeyState(VK_CONTROL) & 0x8000)
	{
		if (GetKeyState('S') & 0x8000)
		{
			Saver->SaveScene(CurrentScene);
		}
	}
}
void EditorWindow::SaveScene()
{
	Saver->SaveScene(CurrentScene);
}
void EditorWindow::LoadScene()
{
	Renderer->SetScene(nullptr);
	delete CurrentScene;
	CurrentScene = new Scene();
	Saver->LoadScene(CurrentScene);
	Renderer->SetScene(CurrentScene);
	UI->UpdateGameObjectList(CurrentScene->GetObjects());
	UI->RefreshGameObjectList();
	UI->AlertBox("Scene Loaded");
}
void EditorWindow::ProcessMenu(WORD command)
{
	switch (command)
	{
	case 4://add gameobject
		CurrentScene->AddGameobjectToScene(new GameObject());
		UI->UpdateGameObjectList(CurrentScene->GetObjects());
		UI->RefreshGameObjectList();
		break;
	case 5://Save Scene
		SaveScene();
		break;
	case 6://Load Scene
		LoadScene();
		break;
	}
}

void EditorWindow::WindowUI()
{
	std::stringstream stream;
	//todo: Move to Game
	//std::string shape = "Box";
	//if (input->GetShape() == 1)
	//{
	//	shape = "Sphere";
	//}
	//stream << "Current force " << (input->GetForce()) << std::setprecision(6) << " Shape = " << shape;
	//UI->RenderTextToScreen(2, stream.str());
	//stream.str("");

	std::string go = "";
	std::string goname = "";
	/*if (UI->IsUIBlocking())
	{
		stream << "Blocking  "; 
	}*/
	if (input->Selectedobject != nullptr)
	{
		go = glm::to_string(input->Selectedobject->GetTransform()->GetPos());
		goname = input->Selectedobject->GetName();
	}
	stream << (input->currentObjectIndex) << " Obj " << goname << " Position " << go << std::setprecision(3);
	UI->RenderTextToScreen(3, stream.str());

	stream.str("");
	statcount++;
	
	int totalmem;
	int freemem = 0;
	if (statcount > 40)
	{
		glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &totalmem);
		glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &freemem);
		currentmemeory = totalmem - freemem;
		statcount = 0;
	}
	stream << "Current memeory " << (float)(currentmemeory) / 1024.0f << "Mb ";
	UI->RenderTextToScreen(4, stream.str());
}

