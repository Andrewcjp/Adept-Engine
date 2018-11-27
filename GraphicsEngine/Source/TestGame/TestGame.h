#pragma once
#include "Core/Game/Game.h"
#include "TGExtraComponentRegister.h"
#include "TestGameGameMode.h"
class TestGame :
	public Game
{
public:
	TestGame(class CompoenentRegistry* Reg); 
	~TestGame();
	virtual GameMode* CreateGameMode() override;
	AIDirector * CreateAIDirector() override;

	virtual GameHud* CreateGameHUD() override;

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