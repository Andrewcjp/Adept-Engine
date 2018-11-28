#pragma once
#include "UI/GameUI/GameHud.h"

class UILabel;
class TestGameGameMode;
class TestGameHud : public GameHud
{
public:
	TestGameHud();
	~TestGameHud();

	virtual void OnStart() override;


	virtual void OnUpdate() override;


	virtual void OnDestory() override;
private:
	UILabel* ammoCounter = nullptr;
	TestGameGameMode* Mode;
};

