#pragma once
#include "AI/Core/AIDirector.h"
#include "Core/Types/WeakObjectPtr.h"

namespace EAIType
{
	enum Type
	{
		HellKnight,
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
namespace EWaveStage
{
	enum Type
	{
		Starting,//gore nest just popped
		SecondWaveSpawns,//first wave has been dealt with send more to the meat grinder
		HeavySpawns,//Focus on spawning only heavy units
		EndState,//let the grinder clean up and exit
		Limit
	};
}
struct DirectorState
{
	int MaxScoreSpawned = 0; //the hardest enemy that  can be spawned 
	int WaveSize = 0;
	int NextStageThreshold = 0;//once the current spawned score drops below this the director moves to the next state
};
struct DirectorStateSet
{
	const DirectorState* GetState(EWaveStage::Type t) const
	{
		return &States[t];
	}
	void SetDefault();
private:
	DirectorState States[EWaveStage::Limit];
};
class GameObject;
class AttackController;
class BleedOutGameMode;
class BleedOut_Director : public AIDirector
{
public:
	BleedOut_Director();
	virtual ~BleedOut_Director();
	void Tick() override;
	int GetSpawnedScore();
	void TickNewAIQueue();
	glm::vec3 GetSpawnPos();
	void NotifySpawningPoolDestruction();
	GameObject* SpawnAI(glm::vec3 SpawnPos, EAIType::Type type);
	GameObject * CreateAI(glm::vec3 pos, float AttackRaduis, float BaseRaduis);
	GameObject * SpawnHellKnight(glm::vec3 pos);
	GameObject * SpawnSoldier(glm::vec3 pos);
	GameObject * SpawnRioter(glm::vec3 pos);
	GameObject * SpawnOrb(glm::vec3 pos);
	GameObject * SpawnSkull(glm::vec3 pos);
	void EnqueueWave(const DirectorState* state);
	void TryMoveNextState();
	static std::string GetStateName(EWaveStage::Type state);
	void SetState(EWaveStage::Type newstate);
	AttackController* GetPlayerAttackController();
	int GetMaxAttackingAI();
private:
	struct SpawnedAi
	{
		WeakObjectPtr<GameObject> Object = nullptr;
		EAIType::Type type;
	};
	int GetAiScore(EAIType::Type);
	std::queue<EAIType::Type> IncomingAI;
	std::vector<SpawnedAi*> CurrentlySpawnedAI;
	bool once = false;
	int DifficultyScoreMax = 5;
	int CurrentSpawnScore = 0;
	BleedOutGameMode* GameMode = nullptr;
	DirectorStateSet* StateSets = nullptr;
	EWaveStage::Type CurrnetStage = EWaveStage::Limit;
	AttackController* PlayerAttackController = nullptr;//For single player 
	int SpawnDelay = 60;
	float AISpawnDelay = 0.5f;
	float CurrentSpawnDelay = 0.0f;
};

