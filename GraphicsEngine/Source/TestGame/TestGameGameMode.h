#pragma once
#include "EngineHeader.h"
class TestGameGameMode:public GameMode
{
public:
	TestGameGameMode();
	~TestGameGameMode();
	virtual void BeginPlay();
	virtual void EndPlay();
	virtual void Update();
};

