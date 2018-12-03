#include "Health.h"
#include "Core/GameObject.h"
#include "Audio/AudioEngine.h"


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
	if (GetOwner()->Tags.Contains(Tag("player")))
	{
		AudioEngine::PostEvent("Melee_Hit", GetOwner());
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
