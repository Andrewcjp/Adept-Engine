#include "BleedOut_Director.h"
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
#include "Source/BleedOutGame/Components/Health.h"
#include "Source/BleedOutGame/Components/MeleeWeapon.h"
#include "Core/Performance/PerfManager.h"
#include "AttackController.h"
#include "Core/Platform/ConsoleVariable.h"
BleedOut_Director::BleedOut_Director()
{
	StateSets = new DirectorStateSet();
	StateSets->SetDefault();
	PlayerAttackController = new AttackController();
	PlayerAttackController->OwningDirector = this;
}

BleedOut_Director::~BleedOut_Director()
{
	SafeDelete(PlayerAttackController);
}

void BleedOut_Director::Tick()
{
	if (!once)
	{
		SpawnAI(glm::vec3(-5, 20, 0), EAIType::PossessedSoldier);
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

int BleedOut_Director::GetSpawnedScore()
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

void BleedOut_Director::TickNewAIQueue()
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
glm::vec3 BleedOut_Director::GetSpawnPos()
{//todo: random offsets to avoid stacking
	glm::vec3 offset = glm::vec3(randvalue, 0, 0);
	randvalue += 4.0;
	const int index = GetRand(0, (int)spawnmarkers.size() - 1);
	//todo: cast to check
	return spawnmarkers[index]->GetOwner()->GetPosition() + offset;
}

void BleedOut_Director::NotifySpawningPoolDestruction()
{
	//lock the area
	//Spawn A Wave!
	SetState(EWaveStage::Starting);
}

GameObject* BleedOut_Director::SpawnAI(glm::vec3 SpawnPos, EAIType::Type type)
{
	SCOPE_STARTUP_COUNTER("SpawnAI");
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
	PerfManager::EndAndLogTimer("SpawnAI");
	return NewAi;
}

GameObject * BleedOut_Director::CreateAI(glm::vec3 pos, float AttackRaduis)
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

GameObject* BleedOut_Director::SpawnHellKnight(glm::vec3 pos)
{
	GameObject* newKnight = CreateAI(pos, 4.0f);
	newKnight->SetName("Hell Knight");

	GameObject* MeshC = new GameObject();
	MeshC->SetParent(newKnight);
	MeshC->GetTransform()->SetLocalPosition(glm::vec3(0, -1.5, 0));
	Material* mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("Texture\\Mutant_diffuse.png"));

	MeshLoader::FMeshLoadingSettings AnimSetting;
	AnimSetting.FlipUVs = true;
	MeshRendererComponent* mrc = MeshC->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("AlwaysCook\\Creature NPC Pack\\mutant.fbx", AnimSetting), mat));
	mrc->LoadAnimation("AlwaysCook\\Creature NPC Pack\\mutant swiping.fbx", "Attack");
	mrc->LoadAnimation("AlwaysCook\\Creature NPC Pack\\mutant idle.fbx", "Idle");
	mrc->LoadAnimation("AlwaysCook\\Creature NPC Pack\\mutant run.fbx", "Walking");
	AnimSetting.AnimSettings.Rate = 2.0f;
	mrc->LoadAnimation("AlwaysCook\\Creature NPC Pack\\mutant dying.fbx", "Death", AnimSetting);
	mrc->PlayAnim("Attack");
	MeshC->GetTransform()->SetScale(glm::vec3(0.02f));
	scene->AddGameobjectToScene(MeshC);
	newKnight->AttachComponent(new HellKnight());
	ColliderComponent* cc = newKnight->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eSPHERE);
	cc->Radius = 2.5f;
	cc->LocalOffset = glm::vec3(0, 3, 0);
	return newKnight;
}

GameObject* BleedOut_Director::SpawnSoldier(glm::vec3 pos)
{
	GameObject* NewPossessed = CreateAI(pos, 4.0f);
	NewPossessed->SetName("Possessed Soldier");

	GameObject* MeshC = new GameObject();
	MeshC->SetParent(NewPossessed);
	MeshC->GetTransform()->SetLocalPosition(glm::vec3(0, -1.5, 0));
	Material* mat = Material::GetDefaultMaterial();
	MeshLoader::FMeshLoadingSettings AnimSetting;
	AnimSetting.FlipUVs = true;
	AnimSetting.AllowInstancing = false;
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("Texture\\vanguard_diffuse.png"));
	MeshRendererComponent* mrc = MeshC->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("AlwaysCook\\Possessed\\vanguard.fbx", AnimSetting), mat));
	//mrc->LoadAnimation("AlwaysCook\\Possessed\\Rifle Walk.fbx", "Attack");
	mrc->LoadAnimation("AlwaysCook\\Possessed\\Idle Aiming.fbx", "Idle");
	//mrc->LoadAnimation("AlwaysCook\\Possessed\\vanguard_t_choonyung@Idle.fbx", "Idle");
	mrc->LoadAnimation("AlwaysCook\\Possessed\\Rifle Walk.fbx", "Walking");
	mrc->LoadAnimation("AlwaysCook\\Possessed\\Death.fbx", "Death");//Rifle Punch
	mrc->LoadAnimation("AlwaysCook\\Possessed\\Rifle Punch.fbx", "Melee");

	mrc->PlayAnim("Idle");
	MeshC->GetTransform()->SetScale(glm::vec3(0.015f));
	scene->AddGameobjectToScene(MeshC);
	GameObject* WeaponBone = new GameObject();
	PossessedSoldier* t = NewPossessed->AttachComponent(new PossessedSoldier());
	t->WeaponBone = WeaponBone;
	t->VisualMesh = MeshC;
	t->MainWeapon = WeaponBone->AttachComponent(new Weapon(Weapon::AIRifle, scene, nullptr, WeaponBone));
	WeaponBone->SetParent(NewPossessed);
	WeaponBone->GetTransform()->SetLocalPosition(glm::vec3(0, 2.5, 0));
	WeaponBone->GetTransform()->SetScale(glm::vec3(0.25));
	scene->AddGameobjectToScene(WeaponBone);
	ColliderComponent* cc = NewPossessed->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eSPHERE);
	cc->Radius = 2.0f;
	cc->LocalOffset = glm::vec3(0, 3, 0);
	return NewPossessed;
}

GameObject* BleedOut_Director::SpawnRioter(glm::vec3 pos)
{
	GameObject* NewRioter = CreateAI(pos, 4.0f);
	NewRioter->AttachComponent(new DemonRiotShield());
	return NewRioter;
}

GameObject* BleedOut_Director::SpawnOrb(glm::vec3 pos)
{
	GameObject* newOrb = CreateAI(pos, 4.0f);
	newOrb->AttachComponent(new DemonOrb());
	return newOrb;
}

GameObject* BleedOut_Director::SpawnSkull(glm::vec3 pos)
{
	GameObject* NewSkullChaser = CreateAI(pos, 4.0f);
	NewSkullChaser->AttachComponent(new SkullChaser());
	return NewSkullChaser;
}

void BleedOut_Director::EnqueueWave(const DirectorState * state)
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

void BleedOut_Director::TryMoveNextState()
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

std::string BleedOut_Director::GetStateName(EWaveStage::Type state)
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

void BleedOut_Director::SetState(EWaveStage::Type newstate)
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

AttackController * BleedOut_Director::GetPlayerAttackController()
{
	return PlayerAttackController;
}
static ConsoleVariable NoAttackMode("NOATK", 1, ECVarType::ConsoleAndLaunch);
int BleedOut_Director::GetMaxAttackingAI()
{
	if (NoAttackMode.GetBoolValue())
	{
		return 0;
	}
	return CurrentPreset.MaxAttackingAI;
}

int BleedOut_Director::GetAiScore(EAIType::Type t)
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
