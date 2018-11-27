#pragma once
#include "UI/GameUI/GameHud.h"
class TestGameHud : public GameHud
{
public:
	TestGameHud();
	~TestGameHud();

	virtual void OnStart() override;


	virtual void OnUpdate() override;


	virtual void OnDestory() override;

};

