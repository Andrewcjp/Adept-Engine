#include "Stdafx.h"
#include "TestGame_Director.h"
#include "Core/GameObject.h"
#include "DemonImp.h"
#include "AI/Core/AIController.h"
#include "Core/Components/RigidbodyComponent.h"
#include "Core/Components/ColliderComponent.h"
#include "DemonRiotShield.h"
#include "DemonOrb.h"
#include "SkullChaser.h"

TestGame_Director::TestGame_Director()
{}


TestGame_Director::~TestGame_Director()
{}

void TestGame_Director::Tick()
{

}

void TestGame_Director::NotifySpawningPoolDestruction()
{
	//lock the area
	//Spawn A Wave!
	
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

	return newAI;
}

GameObject* TestGame_Director::SpawnImp(glm::vec3 pos)
{
	GameObject* newImp = CreateAI(pos);
	newImp->AttachComponent(new DemonImp());
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


