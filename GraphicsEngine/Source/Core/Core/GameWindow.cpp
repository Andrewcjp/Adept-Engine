
#include "GameWindow.h"
#include "EngineInc.h"
#include "Core/Assets/Scene.h"
#include "Rendering/Renderers/RenderEngine.h"
#include "Core/Game/Game.h"
#include "AI/Core/AISystem.h"
#include "UI/UIManager.h"
#include "UI/Core/UIWidgetContext.h"
#include "UI/Core/UIButton.h"
GameWindow::GameWindow()
{}


GameWindow::~GameWindow()
{}
#define SKIP_MM 0
void GameWindow::PostInitWindow(int width, int height)
{
	FrameRateLimit = 60;
	Log::OutS << "Game Loaded" << Log::OutS;

	//LoadScene("\\asset\\scene\\testscene.scene");
	CurrentScene = new Scene();
	CurrentScene->LoadExampleScene(nullptr, false);
	Renderer->SetScene(CurrentScene);
	AISystem::Get()->GenerateMesh();//todo: load in package!
#if SKIP_MM
	Play();
#else
	InitMM();
#endif

}

void GameWindow::InitMM()
{
	MMContext = new UIWidgetContext();
	UI->AddWidgetContext(MMContext);

	const float xSize = 0.1f;
	const float ySize = 0.07f;
	const float Padding = 0.01f;
	UIButton* RestartBtn = new UIButton(0, 0);
	RestartBtn->SetScaled(xSize, ySize, 0.1f - xSize / 2, 0.5f - ySize);
	RestartBtn->BindTarget(std::bind(&GameWindow::Play, this));
	RestartBtn->SetText("Play");
	MMContext->AddWidget(RestartBtn);

	UIButton *ExitBtn = new UIButton(0, 0);
	ExitBtn->SetScaled(xSize, ySize, 0.1f - xSize / 2, 0.5f - (ySize + Padding) * 2);
	ExitBtn->BindTarget(std::bind(&GameWindow::ExitGame, this));
	ExitBtn->SetText("Quit");
	MMContext->AddWidget(ExitBtn);
	MMContext->DisplayPause();
}
void GameWindow::ExitGame()
{
	Engine::RequestExit(0);
}

void GameWindow::Play()
{
#if !SKIP_MM
	UI->RemoveWidgetContext(MMContext);
#endif
	IsRunning = true;
	ShouldTickScene = true;
	Engine::GetGame()->BeginPlay();
	CurrentScene->StartScene();
}

void GameWindow::ProcessMenu(unsigned short command)
{}

