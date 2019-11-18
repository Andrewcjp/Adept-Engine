
#include "GameWindow.h"
#include "EngineInc.h"
#include "Core/Assets/Scene.h"

#include "Core/Game/Game.h"
#include "AI/Core/AISystem.h"
#include "UI/UIManager.h"
#include "UI/Core/UIWidgetContext.h"
#include "UI/CompoundWidgets/UIButton.h"
#include "UI/BasicWidgets/UILabel.h"
#include "Version.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Platform/PlatformCore.h"
GameWindow::GameWindow()
{}


GameWindow::~GameWindow()
{}
#define SKIP_MM 1
void GameWindow::PostInitWindow(int width, int height)
{
	SetFrameRateLimit(0);
	Log::OutS << "Game Loaded" << Log::OutS;
#if BUILD_SHIPPING
	PlatformWindow::Maximize();
#endif
	//LoadScene("\\asset\\scene\\testscene.scene");
	CurrentScene = new Scene();
	CurrentScene->LoadExampleScene();
	SceneRenderer::Get()->SetScene(CurrentScene);
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
	UILabel* VersionText = new UILabel("Version " + std::string(Version::GetFullVersionString()), 0, 0, 0, 0);
	VersionText->SetScaled(0.01f, 0.01f, 0.0f, 0.0f);
	MMContext->AddWidget(VersionText);

	const float xSize = 0.1f;
	const float ySize = 0.07f;
	const float Padding = 0.01f;
	PlayBtn = new UIButton(0, 0);
	PlayBtn->SetScaled(xSize, ySize, 0.1f - xSize / 2, 0.5f - ySize);
	PlayBtn->BindTarget(std::bind(&GameWindow::DisplayDiffculty, this));
	PlayBtn->SetText("Play");
	MMContext->AddWidget(PlayBtn);

	ExitBtn = new UIButton(0, 0);
	ExitBtn->SetScaled(xSize, ySize, 0.1f - xSize / 2, 0.5f - (ySize + Padding) * 2);
	ExitBtn->BindTarget(std::bind(&GameWindow::ExitGame, this));
	ExitBtn->SetText("Quit");
	MMContext->AddWidget(ExitBtn);
	MMContext->DisplayPause();


	const float DiffXsize = 0.1f;
	const float DiffYsize = 0.2f;
	const float StartPos = 0.35f - DiffXsize / 2;
	const float Offset = 0.15f;
	EasyButton = new UIButton(0, 0);
	EasyButton->SetScaled(DiffXsize, DiffYsize, StartPos, 0.5f - DiffYsize);
	EasyButton->BindTarget(std::bind(&GameWindow::SetEasy, this));
	EasyButton->SetText("Easy");
	MMContext->AddWidget(EasyButton);

	MeduimButton = new UIButton(0, 0);
	MeduimButton->SetScaled(DiffXsize, DiffYsize, StartPos + Offset, 0.5f - DiffYsize);
	MeduimButton->BindTarget(std::bind(&GameWindow::SetMeduim, this));
	MeduimButton->SetText("Medium");
	MMContext->AddWidget(MeduimButton);

	HardButton = new UIButton(0, 0);
	HardButton->SetScaled(DiffXsize, DiffYsize, StartPos + Offset * 2, 0.5f - DiffYsize);
	HardButton->BindTarget(std::bind(&GameWindow::SetHard, this));
	HardButton->SetText("Hard");
	MMContext->AddWidget(HardButton);

	SetDiffcultyPanelState(false);
}

void GameWindow::SetDiffcultyPanelState(bool state)
{
	HardButton->SetEnabled(state);
	EasyButton->SetEnabled(state);
	MeduimButton->SetEnabled(state);
}

void GameWindow::SetMainPanelState(bool state)
{
	PlayBtn->SetEnabled(state);
	ExitBtn->SetEnabled(state);
}

void GameWindow::DisplayDiffculty()
{
	SetDiffcultyPanelState(true);
	SetMainPanelState(false);
}

void GameWindow::SetDifficulty(EGameDifficulty::Type type)
{
	CurrentScene->GetGameMode()->SetDiffcultyLevel(type);
	Play();
}

void GameWindow::SetEasy()
{
	SetDifficulty(EGameDifficulty::Easy);
}

void GameWindow::SetMeduim()
{
	SetDifficulty(EGameDifficulty::Normal);
}

void GameWindow::SetHard()
{
	SetDifficulty(EGameDifficulty::Hard);
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
