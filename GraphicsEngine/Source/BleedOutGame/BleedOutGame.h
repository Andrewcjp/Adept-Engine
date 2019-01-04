#pragma once
#include "Core/Game/Game.h"
#include "BleedOutExtraComponentRegister.h"
#include "BleedOutGameMode.h"
class BleedOutGame :
	public Game
{
public:
	BleedOutGame(class CompoenentRegistry* Reg); 
	~BleedOutGame();
	virtual GameMode* CreateGameMode() override;
	AIDirector * CreateAIDirector() override;

	virtual GameHud* CreateGameHUD() override;

};
   
 
class BleedOutGameModule :public GameModule
{
	Game* GetGamePtr(CompoenentRegistry* Reg) override
	{
		return new BleedOutGame(Reg);
	} 
};
 
#ifdef STATIC_MODULE
IMPLEMENT_MODULE_STATIC(BleedOutGameModule, BleedOutGame);
#else
IMPLEMENT_MODULE_DYNAMIC(BleedOutGameModule);
#endif