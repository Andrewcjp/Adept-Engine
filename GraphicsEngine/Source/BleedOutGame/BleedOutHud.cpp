#include "Source/Core/Stdafx.h"
#include "BleedOutHud.h"
#include "UI/UIManager.h"
#include "UI/Core/UIBox.h"
#include "UI/GameUI/GameHud.h"
#include "UI/Core/UIWidgetContext.h"
#include "UI/Core/UILabel.h"
#include "BleedOutGameMode.h"
#include "Components/BleedOutPlayer.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "UI/Core/UIButton.h"
#include "Components/Health.h"


BleedOutHud::BleedOutHud()
{

}


BleedOutHud::~BleedOutHud()
{}

void BleedOutHud::OnStart()
{
	Mode = (BleedOutGameMode*)gameMode;
	ammoCounter = new UILabel("0/0", 0, 0, 0, 0);
	ammoCounter->SetScaled(0.2f, 0.2f);
	ammoCounter->TextScale = 0.45f;
	Context->AddWidget(ammoCounter);
	
	InteractText = new UILabel("E To Interact", 0, 0, 0, 0);
	InteractText->SetScaled(0.2f, 0.2f, 0.45f, 0.3f);
	InteractText->TextScale = 0.5f;
	Context->AddWidget(InteractText);

	ObjectiveText = new UILabel("Objective: Reach the exit before time runs out", 0, 0, 0, 0);
	ObjectiveText->SetScaled(0.2f, 0.2f, 0.3f, 0.6f);
	ObjectiveText->TextScale = 0.65f;
	Context->AddWidget(ObjectiveText);

	ResumeBtn = new UIButton(0, 0);
	const float xSize = 0.1f;
	const float ySize = 0.03f;
	ResumeBtn->SetScaled(xSize, ySize, 0.5f - xSize / 2, 0.5f);
	ResumeBtn->BindTarget(std::bind(&BleedOutHud::UnPause, this));
	ResumeBtn->SetText("Resume");
	Context->AddWidget(ResumeBtn);

	RestartBtn = new UIButton(0, 0);
	RestartBtn->SetScaled(xSize, ySize, 0.5f - xSize / 2, 0.5f - ySize);
	RestartBtn->BindTarget(std::bind(&BleedOutHud::Restart, this));
	RestartBtn->SetText("Restart");
	Context->AddWidget(RestartBtn);

	ExitBtn = new UIButton(0, 0);
	ExitBtn->SetScaled(xSize, ySize, 0.5f - xSize / 2, 0.5f - ySize * 2);
	ExitBtn->BindTarget(std::bind(&BleedOutHud::CloseGame, this));
	ExitBtn->SetText("Quit");
	Context->AddWidget(ExitBtn);
	ShowInteractPrompt(false);
	DisplayText("Objective: Reach the exit before time runs out", 3.0f);
}

void BleedOutHud::UnPause()
{
	gameMode->SetPauseState(false);
}

void BleedOutHud::CloseGame()
{
	Engine::RequestExit(0);
}

void BleedOutHud::Restart()
{
	gameMode->RestartLevel();
}

void BleedOutHud::OnUpdate()
{
	if (Mode->GetPlayer() != nullptr)
	{
		BleedOutPlayer* player = Mode->GetPlayer()->GetComponent<BleedOutPlayer>();
		const glm::vec3 UIColour = player->GetColour();
		ammoCounter->SetEnabled(true);
		if (player != nullptr)
		{
			ObjectiveText->Colour = UIColour;
			ammoCounter->Colour = UIColour;
			ammoCounter->SetText(player->GetInfoString());
			if (player->GetPlayerHealth() <= 20)
			{
				DisplayText("LowHealth", 0.0016f);
			}
		}
		const float Crosshairsize = 30.0f;
		const float CentreExclude = 7.0f;
		if (!gameMode->IsGamePaused())
		{
			const glm::vec3 ScreenCentre = glm::vec3(Context->Offset, 0) + glm::vec3(Context->GetWidth() / 2, Context->GetHeight() / 2, 0);
			Context->GetLineBatcher()->AddLine(glm::vec3(1, 0, 0)*CentreExclude + ScreenCentre, ScreenCentre + glm::vec3(1, 0, 0) * Crosshairsize, UIColour);
			Context->GetLineBatcher()->AddLine(glm::vec3(-1, 0, 0)*CentreExclude + ScreenCentre, ScreenCentre + glm::vec3(-1, 0, 0) * Crosshairsize, UIColour);
			Context->GetLineBatcher()->AddLine(glm::vec3(0, 1, 0)*CentreExclude + ScreenCentre, ScreenCentre + glm::vec3(0, 1, 0)*Crosshairsize, UIColour);
			Context->GetLineBatcher()->AddLine(glm::vec3(0, -1, 0)*CentreExclude + ScreenCentre, ScreenCentre + glm::vec3(0, -1, 0)*Crosshairsize, UIColour);
		}
		if (gameMode->IsGamePaused() && !LastState)
		{
			Context->DisplayPause();
		}
		else if (!gameMode->IsGamePaused() && LastState)
		{
			Context->HideScreen();
		}
		LastState = gameMode->IsGamePaused();
		ResumeBtn->SetEnabled(gameMode->IsGamePaused());
		ExitBtn->SetEnabled(gameMode->IsGamePaused());
		RestartBtn->SetEnabled(gameMode->IsGamePaused());
		DisplayTimeRemaining -= Engine::GetDeltaTime();
		if (DisplayTimeRemaining <= 0)
		{
			ObjectiveText->SetEnabled(false);
		}	
		ShowInteractPrompt(false);
	}
}

void BleedOutHud::ShowInteractPrompt(bool state)
{
	InteractText->SetEnabled(state); 
}

void BleedOutHud::ShowRestart()
{
	ExitBtn->SetEnabled(true);
	RestartBtn->SetEnabled(true);
	ammoCounter->SetEnabled(false);
	Context->DisplayPause();
	Input::SetCursorState(false, true);
}

void BleedOutHud::OnDestory()
{

}

void BleedOutHud::DisplayText(std::string Text, float Time)
{
	DisplayTimeRemaining = Time;
	ObjectiveText->SetText(Text);
	ObjectiveText->SetEnabled(true);
}
