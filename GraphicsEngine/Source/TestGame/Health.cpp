#include "Health.h"
#include "Core/GameObject.h"


Health::Health()
{}


Health::~Health()
{}

void Health::TakeDamage(float amt)
{
	CurrentHealth -= amt;
	if (CurrentHealth <= 0)
	{
		GetOwner()->Destory();
	}
}

void Health::InitComponent()
{
	CurrentHealth = MaxHealth;
}
