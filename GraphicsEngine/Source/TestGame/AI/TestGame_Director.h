#pragma once
#include "AI/Core/AIDirector.h"
struct AIDirector_DifficultyPreset
{
	int WaveEnemyCount = 1;
	//when closer to 1 more high level AI will be spawned
	float SizeWeighting = 0.5f;//normal value;

};
namespace EAIType
{
	enum Type
	{
		Imp,
		PossessedSoldier,
		Rioter,
		Orb,
		Skull,
		Limit
	};
};
namespace EDirectorMode
{
	enum Type
	{
		Passive,
		Wave,
		Limit
	};
}
class GameObject;
class TestGame_Director : public AIDirector
{
public:
	TestGame_Director();
	~TestGame_Director();
	void Tick() override;
	glm::vec3 GetSpawnPos();
	void NotifySpawningPoolDestruction();
	GameObject* SpawnAI(glm::vec3 SpawnPos, EAIType::Type type);
	GameObject * CreateAI(glm::vec3 pos);
	GameObject * SpawnImp(glm::vec3 pos);
	GameObject * SpawnSoldier(glm::vec3 pos);
	GameObject * SpawnRioter(glm::vec3 pos);
	GameObject * SpawnOrb(glm::vec3 pos);
	GameObject * SpawnSkull(glm::vec3 pos);
private:
	AIDirector_DifficultyPreset CurrentPreset = AIDirector_DifficultyPreset();
	bool once = false;

};

