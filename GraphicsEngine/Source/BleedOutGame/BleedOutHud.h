#pragma once
#include "UI/GameUI/GameHud.h"

class UILabel;
class BleedOutGameMode;
class UIButton;
class BleedOutHud : public GameHud
{
public:
	BleedOutHud();
	~BleedOutHud();

	virtual void OnStart() override;
	void UnPause();
	void CloseGame();
	void Restart();
	virtual void OnUpdate() override;
	void ShowInteractPrompt(bool state);
	void ShowRestart();
	virtual void OnDestory() override;
	void DisplayText(std::string Test, float Time, float Xoffset);
private:
	UILabel* ObjectiveText = nullptr;
	UILabel* InteractText = nullptr;
	UILabel* ammoCounter = nullptr;
	BleedOutGameMode* Mode;
	UIButton* ResumeBtn = nullptr;
	UIButton* ExitBtn = nullptr;
	UIButton* RestartBtn = nullptr;
	bool LastState = false;
	float DisplayTimeRemaining = 0.0f;
};

