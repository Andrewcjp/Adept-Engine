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
	CORE_API virtual bool IsGamePaused();
	CORE_API virtual void OnPause();
	CORE_API virtual void OnUnPause();
	CORE_API virtual void AlwaysUpdate();
	CORE_API void SetPauseState(bool state);
	CORE_API void RestartLevel();
protected:
	Scene* CurrentScene = nullptr;
	GameHud* Hud = nullptr;

};

