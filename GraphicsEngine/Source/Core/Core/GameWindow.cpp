
#include "GameWindow.h"
#include "EngineInc.h"
#include "Core/Assets/Scene.h"
#include "Rendering/Renderers/RenderEngine.h"
#include "Core/Game/Game.h"
#include "AI/Core/AISystem.h"
GameWindow::GameWindow()
{}


GameWindow::~GameWindow()
{}

void GameWindow::PostInitWindow(int width, int height)
{
	//FrameRateLimit = 60;
	Log::OutS  << "Game Loaded" << Log::OutS;
	IsRunning = true;
	ShouldTickScene = true;
	//LoadScene("\\asset\\scene\\testscene.scene");
	CurrentScene = new Scene();
	CurrentScene->LoadExampleScene(nullptr,false);
	Renderer->SetScene(CurrentScene);
	AISystem::Get()->GenerateMesh();//todo: load in package!
	Engine::GetGame()->BeginPlay();
	CurrentScene->StartScene();
}

void GameWindow::ProcessMenu(unsigned short command)
{}
