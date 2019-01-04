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
	void ShowRestart();
	virtual void OnDestory() override;
private:
	UILabel* ammoCounter = nullptr;
	BleedOutGameMode* Mode;
	UIButton* ResumeBtn = nullptr;
	UIButton* ExitBtn = nullptr;
	UIButton* RestartBtn = nullptr;
	bool LastState = false;
};

