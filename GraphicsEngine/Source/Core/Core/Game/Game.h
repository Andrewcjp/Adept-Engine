#pragma once

#include "Core/Module/ModuleInterface.h"
#include "Core/Platform/Logger.h"
#include "Core/Game/Gamemode.h"
class AIDirector;
class GameHud;
class  Game
{
public:
	CORE_API Game();
	CORE_API virtual ~Game();
	CORE_API virtual void  Update();
	CORE_API  void BeginPlay();
	CORE_API  void EndPlay();
	CORE_API float GetTickRate();
	CORE_API void SetTickRate(float NewRate);
	CORE_API virtual GameMode* CreateGameMode();
	CORE_API virtual AIDirector* CreateAIDirector();
	CORE_API virtual GameHud* CreateGameHUD();
	CORE_API virtual std::vector<std::string> GetPreLoadAssets();
	void Init();
private:
	float TickRate = 60.0f;


};
class GameModule :public IModuleInterface
{
public:
	bool StartupModule() override
	{
		//	Log::OutS << "Game Module Startup" << Log::OutS;
		return true;
	}
	virtual Game* GetGamePtr()
	{
		return nullptr;
	}
};
