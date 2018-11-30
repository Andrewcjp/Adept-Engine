#include "TestGame_Director.h"
#include "AI/Core/AIController.h"
#include "Core/Components/ColliderComponent.h"
#include "Core/Components/MeshRendererComponent.h"
#include "Core/Components/RigidbodyComponent.h"
#include "DemonImp.h"
#include "DemonOrb.h"
#include "DemonRiotShield.h"
#include "SkullChaser.h"
#include "Source/TestGame/Components/Health.h"
#include "AI/Core/AIDirector.h"
#include "Core/Components/Component.h"
#include "AI/Core/SpawnMarker.h"
#include "../Components/MeleeWeapon.h"

TestGame_Director::TestGame_Director()
{}


TestGame_Director::~TestGame_Director()
{}

void TestGame_Director::Tick()
{
	if (!once)
	{
		SpawnAI(glm::vec3(50, 10, 0), EAIType::Imp);
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
	SpawnAI(GetSpawnPos(), EAIType::Imp);
}

GameObject* TestGame_Director::SpawnAI(glm::vec3 SpawnPos, EAIType::Type type)
{
	switch (type)
	{
	case EAIType::Imp:
		return SpawnImp(SpawnPos);
	case EAIType::Rioter:
		return SpawnRioter(SpawnPos);
	case EAIType::Orb:
		return SpawnOrb(SpawnPos);
	case EAIType::Skull:
		return SpawnSkull(SpawnPos);
	}
	return nullptr;
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
	newImp->AttachComponent(new DemonImp());
	Material* mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	newImp->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Model test.obj"), mat));
	newImp->GetTransform()->SetScale(glm::vec3(1, 2, 1));
	scene->AddGameobjectToScene(newImp);
	return newImp;
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


