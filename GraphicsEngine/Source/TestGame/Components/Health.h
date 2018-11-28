#pragma once
class Health : public Component
{
public:
	Health();
	~Health();
	void TakeDamage(float amt);
	float MaxHealth = 100.0f;
	float GetCurrentHealth() { return CurrentHealth; }
	void AddHealth(float amt);
private:
	
	float CurrentHealth = 100.0f;

	// Inherited via Component
	virtual void SceneInitComponent() override;


	// Inherited via Component
	virtual  void InitComponent() override;

};

