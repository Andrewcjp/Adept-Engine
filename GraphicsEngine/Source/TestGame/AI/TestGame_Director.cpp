#include "TestGame_Director.h"
#include "AI/Core/AIController.h"
#include "AI/Core/AIDirector.h"
#include "AI/Core/SpawnMarker.h"
#include "Core/Components/ColliderComponent.h"
#include "Core/Components/Component.h"
#include "Core/Components/MeshRendererComponent.h"
#include "Core/Components/RigidbodyComponent.h"
#include "Source/TestGame/AI/HellKnight/HellKnight.h"
#include "DemonOrb.h"
#include "DemonRiotShield.h"
#include "SkullChaser.h"
#include "Source/TestGame/Components/Health.h"
#include "Source/TestGame/Components/MeleeWeapon.h"
#include "PossessedSoldier/PossessedSoldier.h"

TestGame_Director::TestGame_Director()
{}


TestGame_Director::~TestGame_Director()
{}

void TestGame_Director::Tick()
{
	if (!once)
	{
	//	SpawnAI(glm::vec3(50, 10, 0), EAIType::Imp);
		SpawnAI(GetSpawnPos() + glm::vec3(0, 0, 3), EAIType::PossessedSoldier);
		once = true;
	}
}

glm::vec3 TestGame_Director::GetSpawnPos()
{
	return spawnmarkers[0]->GetOwner()->GetPosition();
}

void TestGame_Director::NotifySpawningPoolDestruction()
{
	//lock the area
	//Spawn A Wave!
	//SpawnAI(GetSpawnPos(), EAIType::Imp);
	SpawnAI(GetSpawnPos() + glm::vec3(0, 0, 3), EAIType::PossessedSoldier);
}

GameObject* TestGame_Director::SpawnAI(glm::vec3 SpawnPos, EAIType::Type type)
{
	GameObject* NewAi = nullptr;
	switch (type)
	{
	case EAIType::Imp:
		NewAi = SpawnImp(SpawnPos);
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

GameObject* TestGame_Director::CreateAI(glm::vec3 pos)
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
	cc->Radius = 4.0f;
	mw->Collider = cc;
	return newAI;
}

GameObject* TestGame_Director::SpawnImp(glm::vec3 pos)
{
	GameObject* newImp = CreateAI(pos);
	newImp->SetName("IMP");
	newImp->GetTransform()->SetScale(glm::vec3(2, 1, 1));
	newImp->AttachComponent(new HellKnight());
	Material* mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	newImp->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Model test.obj"), mat));
	newImp->GetTransform()->SetScale(glm::vec3(1, 2, 1));

	return newImp;
}

GameObject* TestGame_Director::SpawnSoldier(glm::vec3 pos)
{
	GameObject* NewPossessed = CreateAI(pos);
	PossessedSoldier* t = NewPossessed->AttachComponent(new PossessedSoldier());
	t->MainWeapon = NewPossessed->AttachComponent(new Weapon(Weapon::Rifle, scene, nullptr));
	Material* mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	NewPossessed->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Model test.obj"), mat));
	NewPossessed->GetTransform()->SetScale(glm::vec3(1, 1, 1));
	return NewPossessed;
}

GameObject* TestGame_Director::SpawnRioter(glm::vec3 pos)
{
	GameObject* NewRioter = CreateAI(pos);
	NewRioter->AttachComponent(new DemonRiotShield());
	return NewRioter;
}

GameObject* TestGame_Director::SpawnOrb(glm::vec3 pos)
{
	GameObject* newOrb = CreateAI(pos);
	newOrb->AttachComponent(new DemonOrb());
	return newOrb;
}

GameObject* TestGame_Director::SpawnSkull(glm::vec3 pos)
{
	GameObject* NewSkullChaser = CreateAI(pos);
	NewSkullChaser->AttachComponent(new SkullChaser());
	return NewSkullChaser;
}


