#pragma once
#include "Core/Components/Core_Components_FWD.h"
#include "Core/Components/Component.h"
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
	};
	const WeaponSettings& GetCurrentSettings() { return CurrentSettings; }
	void SetCurrentSettings(WeaponSettings NewSettings);
	void Fire();
private:
	float CurrentCoolDown = 0.0f;
	float CurrentFireRate = 0.1f;
	WeaponType CurrentWeaponType = WeaponType::Rifle;
	WeaponSettings CurrentSettings = WeaponSettings();
	float ProjectileSpeed = 75.0f;
};
