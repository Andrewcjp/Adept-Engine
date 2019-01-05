#pragma once

class Health : public Component
{
public:
	Health();
	~Health();
	void TakeDamage(float amt, bool BlockSound = false);
	float MaxHealth = 100.0f;
	float GetCurrentHealth()
	{
		return CurrentHealth;
	}
	void AddHealth(float amt);

	void BindDeathCallback(std::function<void()> t)
	{
		DeathCallback = t;
	}
	bool Damageable = true;
private:
	std::function<void()> DeathCallback;
	float CurrentHealth = 100.0f;
	bool DeathCalled = false;
	// Inherited via Component
	virtual void SceneInitComponent() override;


	// Inherited via Component
	virtual  void InitComponent() override;

};

