#include "Health.h"
#include "Core/GameObject.h"
#include "Audio/AudioEngine.h"


Health::Health()
{}


Health::~Health()
{}

void Health::TakeDamage(float amt)
{
	if (DeathCalled)
	{
		Log::LogMessage("Hit dead object");
		return;
	}
	std::stringstream ss;
	ss << "object: " << GetOwner()->GetName() << " took damage " << amt;
	Log::LogMessage(ss.str());
	CurrentHealth -= amt;
	if (CurrentHealth <= 0 && Damageable)
	{
		if (DeathCallback != nullptr)
		{
			DeathCallback();
		}
		else
		{
			GetOwner()->Destory();
		}
		DeathCalled = true;
	}
	if (GetOwner()->Tags.Contains(Tag("player")))
	{
		AudioEngine::PostEvent("Melee_Hit", GetOwner());
	}
	if (!Damageable)
	{
		CurrentHealth = MaxHealth;
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
