#pragma once

class UIWidgetContext;
class GameMode;
class GameHud
{
public:
	CORE_API GameHud();
	CORE_API virtual ~GameHud();
	void Start();
	void Update();
	void Destory();
	void SetGameMode(GameMode* gm);
protected:
	CORE_API virtual void OnStart();
	CORE_API virtual void OnUpdate();
	CORE_API virtual void OnDestory();
	UIWidgetContext* Context = nullptr;
	GameMode* gameMode = nullptr;
};

 