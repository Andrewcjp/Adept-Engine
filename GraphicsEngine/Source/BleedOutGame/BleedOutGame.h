#pragma once
#include "Core/Game/Game.h"
#include "BleedOutExtraComponentRegister.h"
#include "BleedOutGameMode.h"
class BleedOutGame :
	public Game
{
public:
	BleedOutGame(class ComponentRegistry* Reg);
	~BleedOutGame();
	virtual GameMode* CreateGameMode() override;
	AIDirector * CreateAIDirector() override;

	virtual GameHud* CreateGameHUD() override;
	std::vector<std::string> GetPreLoadAssets() override;
};


class BleedOutGameModule :public GameModule
{
	Game* GetGamePtr(ComponentRegistry* Reg) override
	{
		return new BleedOutGame(Reg);
	}
};

#ifdef STATIC_MODULE
IMPLEMENT_MODULE_STATIC(BleedOutGameModule, BleedOutGame);
#else
IMPLEMENT_MODULE_DYNAMIC(BleedOutGameModule);
#endif