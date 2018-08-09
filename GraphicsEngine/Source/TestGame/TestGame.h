#pragma once
#include "Core\Game.h"
#include "TGExtraComponentRegister.h"

class TestGame :
	public Game
{
public:
	TestGame(class CompoenentRegistry* Reg); 
	~TestGame();
};
   

class TestGameModule :public GameModule
{
	Game* GetGamePtr(CompoenentRegistry* Reg) override
	{
		return new TestGame(Reg);
	}
};

#ifdef STATIC_MODULE
IMPLEMENT_MODULE_STATIC(TestGameModule, TestGame);
#else
IMPLEMENT_MODULE_DYNAMIC(TestGameModule);
#endif