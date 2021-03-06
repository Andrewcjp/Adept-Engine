#include "B_AIBase.h"
#include "Core/GameObject.h"
#include "Core/Utils/MathUtils.h"
#include "Components/Health.h"
#include "Components/Weapon.h"

#include "AI/Core/AISystem.h"
#include "BleedOut_Director.h"
#include "../BleedOutGameMode.h"
#include "Core/Components/RigidbodyComponent.h"


B_AIBase::B_AIBase()
{}


B_AIBase::~B_AIBase()
{}

bool B_AIBase::FireAt(glm::vec3 pos)
{
	//detect angle delta 
	//fire x shots
	if (MainWeapon->GetOwner() != GetOwner())
	{
		glm::vec3 direction = MainWeapon->GetOwner()->GetPosition() - pos;
		direction = MathUtils::SafeNormalise(direction);
		//MainWeapon->GetOwner()->GetTransform()->SetLocalRotation(glm::quat(0, 1, 0, 0));
	}
	return MainWeapon->Fire();
}

void B_AIBase::OnDead()
{
	AIBase::SetDead();
	Health* H = GetOwner()->GetComponent<Health>();
	if (H != nullptr)
	{
		float AMt = H->MaxHealth *GameMode->GetDifficultyPreset()->AITransferPC;
		if (GameMode->GetPlayer() != nullptr)
		{
			H = GameMode->GetPlayer()->GetComponent<Health>();
			H->AddHealth(AMt);
		}
	}
	if (Mesh != nullptr)
	{
		Mesh->GetTransform()->SetLocalPosition(DeathOffset);
	}
}

void B_AIBase::SceneInitComponent()
{
	AIBase::SceneInitComponent();
	GameMode = (BleedOutGameMode*)GetOwner()->GetScene()->GetGameMode();
}

void B_AIBase::Update(float dt)
{
	AIBase::Update(dt);
	if (RB != nullptr)
	{
		Walk.Tick(GetOwner(), true, RB->GetVelocity(), true);
	}
}

void B_AIBase::InitComponent()
{
	AIBase::InitComponent();
	MainWeapon = GetOwner()->GetComponent<Weapon>();
	Health* H = GetOwner()->GetComponent<Health>();
	H->BindDeathCallback(std::bind(&B_AIBase::OnDead, this));
	RB = GetOwner()->GetComponent<RigidbodyComponent>();
}
