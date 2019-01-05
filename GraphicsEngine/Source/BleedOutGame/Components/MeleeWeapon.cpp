#include "MeleeWeapon.h"
#include "Core/Components/ColliderComponent.h"
#include "Core/Platform/PlatformCore.h"
#include "Health.h"
#include "Audio/AudioEngine.h"
#include "../BleedOutGameMode.h"

MeleeWeapon::MeleeWeapon()
{}

MeleeWeapon::~MeleeWeapon()
{}

bool MeleeWeapon::Fire()
{
	if (CurrentAttackTime > 0.0f)
	{
		return false;
	}
	if (CurrentAttackCoolDown > 0.0f)
	{
		return false;
	}
	AudioEngine::PostEvent("Melee_Move", GetOwner());
	Collider->SetEnabled(true);
	CurrentAttackTime = AttackLength;
	CurrentAttackCoolDown = AttackCooldown;
	return true;
}

void MeleeWeapon::Update(float delta)
{
	Weapon::Update(delta);
	CurrentAttackCoolDown -= delta;
	if (CurrentAttackTime >= 0.0f)
	{
		CurrentAttackTime -= delta;
	}
	else if (Collider != nullptr)
	{
		Collider->SetEnabled(false);
	}
}

void MeleeWeapon::OnTrigger(CollisonData data)
{
	if (Collider == nullptr)
	{
		return;
	}
	if (data.OtherCollider->IsTrigger)
	{
		return;
	}
	Health* h = data.OtherCollider->GetGameObject()->GetComponent<Health>();
	if (h != nullptr && h->GetOwner() != GetOwner())
	{
		h->TakeDamage(AttackDamage);
		/*AudioEngine::PostEvent("Melee_Hit", GetOwner());*/
	}
}

void MeleeWeapon::BeginPlay()
{
	if (Collider != nullptr)
	{
		Collider->SetEnabled(false);
	}
	AttackDamage = 50.0f * DifficultyPreset::Get()->MeleeDamageMulti;
}
