#pragma once
#define USE_POSITION_RECOIL 1
class GameObject;
class BleedOutPlayer;
class Weapon : public Component
{
public:
	enum WeaponType
	{
		Rifle,
		RailGun,
		ShotGun,
		AIRifle,
		Limit,
	};
	Weapon()
	{}
	Weapon(Weapon::WeaponType T, Scene* scene, BleedOutPlayer* player, GameObject* root = nullptr);
	~Weapon();

	// Inherited via Component
	virtual void InitComponent() override;
	virtual void Update(float delta) override;
	struct WeaponSettings
	{
		float FireDelay = 0.2f;
		int PelletCount = 1;
		bool IsSemiAuto = false;
		glm::vec3 WeaponRelativePos = glm::vec3(0, 0, 0);
		float DamagePerShot = 10.0f;
		int MaxAmmoCount = 100;
		float ProjectileSpeed = 50.0f;
		bool ShowProjectile = false;
		float Recoil = 1.0f;
		float MaxRecoil = 1.0f;
		float RecoilReduce = 4.5f;
	};
	const WeaponSettings& GetCurrentSettings()
	{
		return CurrentSettings;
	}
	void SetCurrentSettings(WeaponSettings NewSettings);
	void PlayFireSound();
	virtual bool Fire();
	void SetState(bool state);
	virtual void OnFire()
	{};
	int GetCurrentAmmo()
	{
		return CurrentAmmoCount;
	}
	void AddAmmo(int amt);
	void Recoil(float amt);
	glm::vec3 AIForward = glm::vec3(0, 0, 0);
protected:
	BleedOutPlayer* Player = nullptr;

	int CurrentAmmoCount = 10;
private:
	float CurrnetRecoil = 0.0f;
	
	void CreateModel(Scene* s, GameObject* cameraobj);
	float CurrentCoolDown = 0.0f;
	float CurrentFireRate = 0.1f;
	WeaponType CurrentWeaponType = WeaponType::Rifle;
	WeaponSettings CurrentSettings = WeaponSettings();
	GameObject* WeaponModel = nullptr;
	GameObject* WeaponRoot = nullptr;
	glm::vec3 RootPos = glm::vec3(0);
};
