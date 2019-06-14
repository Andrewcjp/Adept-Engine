#pragma once
#include "Core/Game/Game.h"
class TestGame :
	public Game
{
public:
	TestGame(class ComponentRegistry* Reg);
	~TestGame();

};


class TestGameModule :public GameModule
{
	Game* GetGamePtr(ComponentRegistry* Reg) override
	{
		return new TestGame(Reg);
	}
};

#ifdef STATIC_MODULE
IMPLEMENT_MODULE_STATIC(TestGameModule, TestGame);
#else
IMPLEMENT_MODULE_DYNAMIC(TestGameModule);
#endif