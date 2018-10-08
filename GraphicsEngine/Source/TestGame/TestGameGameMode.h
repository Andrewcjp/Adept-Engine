#pragma once
#include "EngineHeader.h"
class TestGameGameMode:public GameMode
{
public:
	TestGameGameMode();
	~TestGameGameMode();
	virtual void BeginPlay(Scene* Scene);
	void SpawnSKull(glm::vec3 Position);
	virtual void EndPlay();
	virtual void Update();
private:
	GameObject* player = nullptr;
};

