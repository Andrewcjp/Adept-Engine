#pragma once
class Scene;
class GameHud;
class GameMode
{
public:
	CORE_API GameMode();
	CORE_API virtual ~GameMode();
	CORE_API virtual void BeginPlay(Scene* Scene);
	CORE_API virtual void EndPlay();
	CORE_API virtual void Update();
	CORE_API GameHud* GetCurrentHudInstance();
protected:
	Scene* CurrentScene = nullptr;
	GameHud* Hud = nullptr;
	
};

