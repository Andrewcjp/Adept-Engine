#include "Projectile.h"
#include "Core/GameObject.h"
#include "Health.h"

Projectile::Projectile()
{

}


Projectile::~Projectile()
{}

void Projectile::InitComponent()
{

}

void Projectile::OnCollide(CollisonData data)
{
	Health* H = data.OtherCollider->GetGameObject()->GetComponent<Health>();
	if (H != nullptr)
	{
		H->TakeDamage(DamageValue);
	}
	GetOwner()->Destory();
}

void Projectile::OnTrigger(CollisonData data)
{
	Health* H = data.OtherCollider->GetGameObject()->GetComponent<Health>();
	if (H != nullptr)
	{
		H->TakeDamage(DamageValue);
	}
	GetOwner()->Destory();
}
