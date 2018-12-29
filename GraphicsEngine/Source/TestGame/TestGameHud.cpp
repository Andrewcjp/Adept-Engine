#include "Source/Core/Stdafx.h"
#include "TestGameHud.h"
#include "UI/UIManager.h"
#include "UI/Core/UIBox.h"
#include "UI/GameUI/GameHud.h"
#include "UI/Core/UIWidgetContext.h"
#include "UI/Core/UILabel.h"
#include "TestGameGameMode.h"
#include "Components/TestPlayer.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "UI/Core/UIButton.h"


TestGameHud::TestGameHud()
{}


TestGameHud::~TestGameHud()
{}

void TestGameHud::OnStart()
{
	Mode = (TestGameGameMode*)gameMode;
	ammoCounter = new UILabel("0/0", 0, 0, 0, 0);
	ammoCounter->SetScaled(0.2f, 0.2f);
	ammoCounter->TextScale = 0.45f;
	Context->AddWidget(ammoCounter);

	ResumeBtn = new UIButton(0, 0);
	const float xSize = 0.1f;
	const float ySize = 0.03f;
	ResumeBtn->SetScaled(xSize, ySize, 0.5f - xSize / 2, 0.5f);
	ResumeBtn->BindTarget(std::bind(&TestGameHud::UnPause, this));
	ResumeBtn->SetText("Resume");
	Context->AddWidget(ResumeBtn);

	RestartBtn = new UIButton(0, 0);
	RestartBtn->SetScaled(xSize, ySize, 0.5f - xSize / 2, 0.5f - ySize);
	RestartBtn->BindTarget(std::bind(&TestGameHud::Restart, this));
	RestartBtn->SetText("Restart");
	Context->AddWidget(RestartBtn);

	ExitBtn = new UIButton(0, 0);
	ExitBtn->SetScaled(xSize, ySize, 0.5f - xSize / 2, 0.5f - ySize * 2);
	ExitBtn->BindTarget(std::bind(&TestGameHud::CloseGame, this));
	ExitBtn->SetText("Quit");
	Context->AddWidget(ExitBtn);
}
void TestGameHud::UnPause()
{
	gameMode->SetPauseState(false);
}
void TestGameHud::CloseGame()
{
	Engine::RequestExit(0);
}

void TestGameHud::Restart()
{
	gameMode->RestartLevel();
}

void TestGameHud::OnUpdate()
{
	if (Mode->GetPlayer() != nullptr)
	{
		TestPlayer* player = Mode->GetPlayer()->GetComponent<TestPlayer>();
		if (player != nullptr)
		{
			ammoCounter->SetText(player->GetInfoString());
		}
		const float Crosshairsize = 30.0f;
		const float CentreExclude = 7.0f;
		glm::vec3 ScreenCentre = glm::vec3(Context->Offset, 0) + glm::vec3(Context->GetWidth() / 2, Context->GetHeight() / 2, 0);
		Context->GetLineBatcher()->AddLine(glm::vec3(1, 0, 0)*CentreExclude + ScreenCentre, ScreenCentre + glm::vec3(1, 0, 0) * Crosshairsize, glm::vec3(1));
		Context->GetLineBatcher()->AddLine(glm::vec3(-1, 0, 0)*CentreExclude + ScreenCentre, ScreenCentre + glm::vec3(-1, 0, 0) * Crosshairsize, glm::vec3(1));
		Context->GetLineBatcher()->AddLine(glm::vec3(0, 1, 0)*CentreExclude + ScreenCentre, ScreenCentre + glm::vec3(0, 1, 0)*Crosshairsize, glm::vec3(1));
		Context->GetLineBatcher()->AddLine(glm::vec3(0, -1, 0)*CentreExclude + ScreenCentre, ScreenCentre + glm::vec3(0, -1, 0)*Crosshairsize, glm::vec3(1));
	}
	ResumeBtn->SetEnabled(gameMode->IsGamePaused());
	ExitBtn->SetEnabled(gameMode->IsGamePaused());
	RestartBtn->SetEnabled(gameMode->IsGamePaused());
}

void TestGameHud::OnDestory()
{

}
