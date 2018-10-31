#pragma once
class GameObject;
class Weapon : public Component
{
public:
	Weapon();
	~Weapon();
	enum WeaponType
	{
		ShotGun,
		Rifle,
		RailGun,
		Limit,
	};
	// Inherited via Component
	virtual void InitComponent() override;
	virtual void Update(float delta) override;
	struct WeaponSettings
	{
		float FireDelay = 0.2f;
		WeaponType Type = WeaponType::Limit;
		int PelletCount = 1;
		bool IsSemiAuto = false;
		glm::vec3 WeaponRelativePos = glm::vec3(0,0,0);
	};
	const WeaponSettings& GetCurrentSettings() { return CurrentSettings; }
	void SetCurrentSettings(WeaponSettings NewSettings);
	void Fire();
	void SetWeaponModel(GameObject* Model,GameObject* cameraobj);
private:
	float CurrentCoolDown = 0.0f;
	float CurrentFireRate = 0.1f;
	WeaponType CurrentWeaponType = WeaponType::Rifle;
	WeaponSettings CurrentSettings = WeaponSettings();
	float ProjectileSpeed = 30.0f;
	GameObject* WeaponModel = nullptr;
};
