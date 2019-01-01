#pragma once
#include "EngineHeader.h"
class TestGameGameMode :public GameMode
{
public:
	TestGameGameMode();
	~TestGameGameMode();
	virtual void BeginPlay(Scene* Scene);
	void SpawnSKull(glm::vec3 Position);
	virtual void EndPlay();
	virtual void Update();
	void OnPlayerDeath();
	GameObject* GetPlayer() { return MPlayer.Get(); };
	void SpawnPlayer(glm::vec3 Pos, Scene* scene);
private:
	WeakObjectPtr<GameObject> MPlayer;
};

