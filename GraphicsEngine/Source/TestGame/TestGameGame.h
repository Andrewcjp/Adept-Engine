#pragma once
#include "Core/Game/Game.h"
class TestGame :
	public Game
{
public:
	TestGame();
	~TestGame();

};


class TestGameModule :public GameModule
{
	Game* GetGamePtr() override
	{
		return new TestGame();
	}
};

#ifdef STATIC_MODULE
IMPLEMENT_MODULE_STATIC(TestGameModule, TestGame);
#else
IMPLEMENT_MODULE_DYNAMIC(TestGameModule);
#endif