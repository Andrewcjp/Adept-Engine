#include "MeleeWeapon.h"
#include "Core/Components/ColliderComponent.h"
#include "Core/Platform/PlatformCore.h"
#include "Health.h"
#include "Audio/AudioEngine.h"

MeleeWeapon::MeleeWeapon()
{}

MeleeWeapon::~MeleeWeapon()
{}

void MeleeWeapon::Fire()
{
	if (CurrentAttackTime > 0.0f)
	{
		return;
	}
	AudioEngine::PostEvent("Melee_Move");
	//do a box cast or something!
	Collider->SetEnabled(true);
	CurrentAttackTime = AttackLength;
}

void MeleeWeapon::Update(float delta)
{
	Weapon::Update(delta);
	if (CurrentAttackTime >= 0.0f)
	{
		//todo: attack time and attack delay
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
	Health* h = data.OtherCollider->GetGameObject()->GetComponent<Health>();
	if (h != nullptr && !data.OtherCollider->GetGameObject()->Tags.Contains(Tag("player")))
	{
		h->TakeDamage(AttackDamage);
		AudioEngine::PostEvent("Melee_Hit");
	}
}

void MeleeWeapon::BeginPlay()
{
	if (Collider != nullptr)
	{
		Collider->SetEnabled(false);
	}
}
