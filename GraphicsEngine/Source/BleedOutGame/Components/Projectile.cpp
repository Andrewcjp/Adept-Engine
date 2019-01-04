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
	LifeTime = 5.0f;
}

void Projectile::OnCollide(CollisonData data)
{
	Health* H = data.OtherCollider->GetGameObject()->GetComponent<Health>();
	if (H != nullptr && H->GetOwner() != Owner)
	{
		H->TakeDamage(DamageValue);
	}
	GetOwner()->Destory();
}

void Projectile::OnTrigger(CollisonData data)
{
	Health* H = data.OtherCollider->GetGameObject()->GetComponent<Health>();
	if (H != nullptr && H->GetOwner() != Owner)
	{
		H->TakeDamage(DamageValue);
	}
	GetOwner()->Destory();
}

void Projectile::Update(float delta)
{
	LifeTime -= delta;
	if (LifeTime <= 0.0f)
	{
		GetOwner()->Destory();
	}
}
