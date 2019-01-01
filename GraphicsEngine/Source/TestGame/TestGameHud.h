#pragma once
#include "UI/GameUI/GameHud.h"

class UILabel;
class TestGameGameMode;
class UIButton;
class TestGameHud : public GameHud
{
public:
	TestGameHud();
	~TestGameHud();

	virtual void OnStart() override;
	void UnPause();
	void CloseGame();
	void Restart();
	virtual void OnUpdate() override;
	void ShowRestart();
	virtual void OnDestory() override;
private:
	UILabel* ammoCounter = nullptr;
	TestGameGameMode* Mode;
	UIButton* ResumeBtn = nullptr;
	UIButton* ExitBtn = nullptr;
	UIButton* RestartBtn = nullptr;
	bool LastState = false;
};

