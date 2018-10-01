#pragma once
class Scene;
class GameMode
{
public:
	CORE_API GameMode();
	CORE_API ~GameMode();
	CORE_API virtual void BeginPlay(Scene* Scene);
	CORE_API virtual void EndPlay();
	CORE_API virtual void Update();
protected:
	Scene* CurrentScene = nullptr;
};

