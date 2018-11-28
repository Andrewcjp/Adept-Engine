#include "Health.h"
#include "Core/GameObject.h"


Health::Health()
{}


Health::~Health()
{}

void Health::TakeDamage(float amt)
{
	std::stringstream ss;
	ss << "object: " << GetOwner()->GetName() << " took damage " << amt;
	Log::LogMessage(ss.str());
	CurrentHealth -= amt;
	if (CurrentHealth <= 0)
	{
		GetOwner()->Destory();
	}
}

void Health::AddHealth(float amt)
{
	CurrentHealth += amt;
	CurrentHealth = glm::clamp(CurrentHealth, 0.0f, MaxHealth);
}

void Health::SceneInitComponent()
{
	CurrentHealth = MaxHealth;
}

void Health::InitComponent()
{

}
