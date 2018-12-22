#include "TestGame_Director.h"
#include "AI/Core/AIController.h"
#include "AI/Core/SpawnMarker.h"
#include "Core/Components/ColliderComponent.h"
#include "Core/Components/MeshRendererComponent.h"
#include "Core/Components/RigidbodyComponent.h"
#include "DemonOrb.h"
#include "DemonRiotShield.h"
#include "HellKnight/HellKnight.h"
#include "PossessedSoldier/PossessedSoldier.h"
#include "SkullChaser.h"
#include "Source/TestGame/Components/Health.h"
#include "Source/TestGame/Components/MeleeWeapon.h"

TestGame_Director::TestGame_Director()
{
	StateSets = new DirectorStateSet();
	StateSets->SetDefault();
}

TestGame_Director::~TestGame_Director()
{}

void TestGame_Director::Tick()
{
	if (!once)
	{
	//	SpawnAI(GetSpawnPos() + glm::vec3(0, 20, 3), EAIType::PossessedSoldier);
		once = true;
	}
	CurrentSpawnScore = GetSpawnedScore();
	if (CurrnetStage != EWaveStage::Limit)
	{
		TryMoveNextState();
		DifficultyScoreMax = 3;
		if (CurrentSpawnScore < DifficultyScoreMax)//limit the AI spawned on a per difficulty level
		{
			TickNewAIQueue();//todo: delay?
		}//currently this will not handle 
	}
}

int TestGame_Director::GetSpawnedScore()
{
	int total = 0;
	if (CurrentlySpawnedAI.size() == 0)
	{
		return 0;
	}
	for (int i = (int)CurrentlySpawnedAI.size() - 1; i >= 0; i--)
	{
		if (CurrentlySpawnedAI[i]->Object.IsValid())
		{
			total += GetAiScore(CurrentlySpawnedAI[i]->type);
		}
		else
		{
			CurrentlySpawnedAI.erase(CurrentlySpawnedAI.begin() + i);
		}
	}
	return total;
}

void TestGame_Director::TickNewAIQueue()
{
	if (IncomingAI.size() == 0)
	{
		return;
	}
	EAIType::Type NewAiType = IncomingAI.front();
	const int NewAIScore = GetAiScore(NewAiType);
	if (CurrentSpawnScore + NewAIScore > DifficultyScoreMax)
	{
		return;//wait until score free for this ai
	}

	IncomingAI.pop();

	SpawnedAi* ai = new SpawnedAi();
	ai->Object = SpawnAI(GetSpawnPos(), NewAiType);
	ai->type = NewAiType;
	CurrentlySpawnedAI.push_back(ai);
}

int GetRand(int min, int max)
{
	return min + (rand() % static_cast<int>(max - min + 1));
}

float randvalue = 1.0f;
glm::vec3 TestGame_Director::GetSpawnPos()
{//todo: random offsets to avoid stacking
	glm::vec3 offset = glm::vec3(randvalue, 0, 0);
	randvalue += 4.0;
	const int index = GetRand(0, (int)spawnmarkers.size() - 1);
	//todo: cast to check
	return spawnmarkers[index]->GetOwner()->GetPosition() + offset;
}

void TestGame_Director::NotifySpawningPoolDestruction()
{
	//lock the area
	//Spawn A Wave!
	SetState(EWaveStage::Starting);
}

GameObject* TestGame_Director::SpawnAI(glm::vec3 SpawnPos, EAIType::Type type)
{
	GameObject* NewAi = nullptr;
	switch (type)
	{
	case EAIType::HellKnight:
		NewAi = SpawnHellKnight(SpawnPos);
		break;
	case EAIType::PossessedSoldier:
		NewAi = SpawnSoldier(SpawnPos);
		break;
	case EAIType::Rioter:
		NewAi = SpawnRioter(SpawnPos);
		break;
	case EAIType::Orb:
		NewAi = SpawnOrb(SpawnPos);
		break;
	case EAIType::Skull:
		NewAi = SpawnSkull(SpawnPos);
		break;
	}
	if (NewAi)
	{
		scene->AddGameobjectToScene(NewAi);
	}
	return NewAi;
}

GameObject * TestGame_Director::CreateAI(glm::vec3 pos, float AttackRaduis)
{
	GameObject* newAI = GameObject::Instantiate(pos);
	newAI->AttachComponent(new RigidbodyComponent());
	ColliderComponent* cc = newAI->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eCAPSULE);
	AIController* Controller = newAI->AttachComponent(new AIController());
	newAI->AttachComponent(new Health());
	MeleeWeapon* mw = newAI->AttachComponent(new MeleeWeapon());
	cc = newAI->AttachComponent(new ColliderComponent());
	cc->IsTrigger = true;
	cc->SetCollisonShape(EShapeType::eSPHERE);
	cc->SetEnabled(false);
	cc->Radius = AttackRaduis;
	mw->Collider = cc;
	return newAI;
}

GameObject* TestGame_Director::SpawnHellKnight(glm::vec3 pos)
{
	GameObject* newImp = CreateAI(pos, 4.0f);
	newImp->SetName("Hell Knight");
	
	GameObject* MeshC = new GameObject();
	MeshC->SetParent(newImp);
	MeshC->GetTransform()->SetLocalPosition(glm::vec3(0, -1.5, 0));
	Material* mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("Creature NPC Pack\\Mutant_diffuse.png"));

	MeshLoader::FMeshLoadingSettings AnimSetting;
	AnimSetting.FlipUVs = true;
	MeshRendererComponent* mrc = MeshC->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Creature NPC Pack\\mutant.fbx", AnimSetting), mat));
	mrc->LoadAnimation("Creature NPC Pack\\mutant swiping.fbx","Attack");
	mrc->LoadAnimation("Creature NPC Pack\\mutant idle.fbx", "Idle");
	mrc->LoadAnimation("Creature NPC Pack\\mutant run.fbx", "Walking");
	AnimSetting.AnimSettings.Rate = 2.0f;
	mrc->LoadAnimation("Creature NPC Pack\\mutant dying.fbx", "Death", AnimSetting);
	mrc->PlayAnim("Attack");
	MeshC->GetTransform()->SetScale(glm::vec3(0.02f));
	scene->AddGameobjectToScene(MeshC);
	newImp->AttachComponent(new HellKnight());
	return newImp;
}

GameObject* TestGame_Director::SpawnSoldier(glm::vec3 pos)
{
	GameObject* NewPossessed = CreateAI(pos, 4.0f);
	NewPossessed->SetName("Possessed Soldier");
	PossessedSoldier* t = NewPossessed->AttachComponent(new PossessedSoldier());

	Material* mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	NewPossessed->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Model test.obj"), mat));
	NewPossessed->GetTransform()->SetScale(glm::vec3(1, 1, 1));
	GameObject* WeaponBone = new GameObject();
	t->MainWeapon = WeaponBone->AttachComponent(new Weapon(Weapon::AIRifle, scene, nullptr, WeaponBone));
	WeaponBone->SetParent(NewPossessed);
	scene->AddGameobjectToScene(WeaponBone);
	return NewPossessed;
}

GameObject* TestGame_Director::SpawnRioter(glm::vec3 pos)
{
	GameObject* NewRioter = CreateAI(pos, 4.0f);
	NewRioter->AttachComponent(new DemonRiotShield());
	return NewRioter;
}

GameObject* TestGame_Director::SpawnOrb(glm::vec3 pos)
{
	GameObject* newOrb = CreateAI(pos, 4.0f);
	newOrb->AttachComponent(new DemonOrb());
	return newOrb;
}

GameObject* TestGame_Director::SpawnSkull(glm::vec3 pos)
{
	GameObject* NewSkullChaser = CreateAI(pos, 4.0f);
	NewSkullChaser->AttachComponent(new SkullChaser());
	return NewSkullChaser;
}

void TestGame_Director::EnqueueWave(const DirectorState * state)
{
	for (int i = 0; i < state->WaveSize; )
	{
		if (state->MaxScoreSpawned > 4)
		{
			i += 4;
			IncomingAI.emplace(EAIType::HellKnight);
		}
		else
		{
			IncomingAI.emplace(EAIType::PossessedSoldier);
			i++;
		}
	}
	randvalue = 0.0f;
	// fill the wave queue with relevant Units
}

void TestGame_Director::TryMoveNextState()
{
	if (IncomingAI.size() > 0)//if AI are still being spawned we are not ready for more yet...
	{
		return;
	}
	const DirectorState* state = StateSets->GetState(CurrnetStage);
	if (CurrentSpawnScore <= state->NextStageThreshold)//has the player killed enough AI to progress to the next wave?
	{
		SetState((EWaveStage::Type)(1 + (int)CurrnetStage));
	}
}

std::string TestGame_Director::GetStateName(EWaveStage::Type state)
{
	switch (state)
	{
	case EWaveStage::Starting:
		return "Starting";
	case EWaveStage::SecondWaveSpawns:
		return "SecondWaveSpawns";
	case EWaveStage::HeavySpawns:
		return "HeavySpawns";
	case EWaveStage::EndState:
		return "EndState";
	default:
		break;
	}
	return "Limit";
}

void TestGame_Director::SetState(EWaveStage::Type newstate)
{
	Log::LogMessage("Moved to state " + GetStateName(newstate));
	CurrnetStage = newstate;
	if (CurrnetStage == EWaveStage::Limit)
	{
		Log::LogMessage("Wave Complete");
	}
	else
	{
		EnqueueWave(StateSets->GetState(CurrnetStage));
	}
}

int TestGame_Director::GetAiScore(EAIType::Type t)
{
	switch (t)
	{
	default:
		break;
	case EAIType::HellKnight:
		return 2;
	case EAIType::PossessedSoldier:
		return 1;

	}
	return 0;
}

void DirectorStateSet::SetDefault()
{
	States[EWaveStage::Starting].MaxScoreSpawned = 1;
	States[EWaveStage::Starting].WaveSize = 2;
	States[EWaveStage::Starting].NextStageThreshold = 2;

	States[EWaveStage::SecondWaveSpawns].MaxScoreSpawned = 2;
	States[EWaveStage::SecondWaveSpawns].WaveSize = 3;
	States[EWaveStage::SecondWaveSpawns].NextStageThreshold = 2;

	States[EWaveStage::HeavySpawns].MaxScoreSpawned = 6;
	States[EWaveStage::HeavySpawns].WaveSize = 6;
	States[EWaveStage::HeavySpawns].NextStageThreshold = 0;

	States[EWaveStage::EndState].MaxScoreSpawned = 0;
	States[EWaveStage::EndState].WaveSize = 0;
	States[EWaveStage::EndState].NextStageThreshold = 0;

}
