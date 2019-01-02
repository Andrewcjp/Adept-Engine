#include "Weapon.h"
#include "Projectile.h"
#include "Audio/AudioEngine.h"
#include "TestPlayer.h"
#include "Core/Utils/DebugDrawers.h"
#include "Core/Performance/PerfManager.h"

Weapon::Weapon(Weapon::WeaponType T, Scene* scene, TestPlayer* player, GameObject* root)
{
	if (T == Rifle)
	{
		CurrentSettings.FireDelay = 0.135f;
		CurrentSettings.DamagePerShot = 20;
		CurrentSettings.MaxAmmoCount = 250;
		CurrentAmmoCount = 250;
	}
	else if (T == ShotGun)
	{
		CurrentSettings.FireDelay = 0.350f;
		CurrentSettings.PelletCount = 5;
		CurrentSettings.DamagePerShot = 20;
		CurrentSettings.MaxAmmoCount = 20;
	}
	else if (T == RailGun)
	{
		CurrentSettings.FireDelay = 0.850f;
		CurrentSettings.DamagePerShot = 150;
		CurrentSettings.MaxAmmoCount = 20;
		CurrentSettings.ProjectileSpeed = 150.0f;
	}
	else if (T == AIRifle)
	{
		CurrentSettings.FireDelay = 0.135f;
		CurrentSettings.DamagePerShot = 10;
		CurrentSettings.MaxAmmoCount = 250;
		CurrentSettings.ShowProjectile = true;
		CurrentAmmoCount = 250;
	}
	Player = player;
	CurrentWeaponType = T;
	if (player != nullptr)
	{
		WeaponRoot = player->CameraObject;
		CreateModel(scene, player->CameraObject);
	}
	else if (root != nullptr)
	{
		WeaponRoot = root;
		CreateModel(scene, root);
	}
}

void Weapon::CreateModel(Scene* s, GameObject* cameraobj)
{
	GameObject* go = new GameObject("Gun Test");
	Material* mat = Material::GetDefaultMaterial();
	go->GetTransform()->SetPos(glm::vec3(0, 2, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	WeaponModel = go;
	WeaponModel->SetParent(cameraobj);
	if (CurrentWeaponType == WeaponType::Rifle || CurrentWeaponType == WeaponType::AIRifle)
	{
		mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("Weapons\\Rifle\\Textures\\Variation 06\\Rifle_06_Albedo.png"));
		MeshLoader::FMeshLoadingSettings set;
		set.FlipUVs = true;
		set.IgnoredMeshObjectNames.push_back("R_Bullet");
		set.IgnoredMeshObjectNames.push_back("R_Sight");
		set.IgnoredMeshObjectNames.push_back("R_Grip");
		go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("AlwaysCook\\Rifle.fbx", set), mat));
		WeaponModel->GetTransform()->SetLocalPosition(glm::vec3(1.2, -1, 2.5));
		WeaponModel->GetTransform()->SetScale(glm::vec3(0.5f));
	}
	else if (CurrentWeaponType == WeaponType::ShotGun)
	{
		//unused!
	}
	else if (CurrentWeaponType == WeaponType::RailGun)
	{
		mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("Weapons\\Heavy\\Textures\\Variation 08\\Heavy_08_Albedo.png"));
		MeshLoader::FMeshLoadingSettings set;
		set.FlipUVs = true;
		set.IgnoredMeshObjectNames.push_back("C_Bullet");
		set.IgnoredMeshObjectNames.push_back("C_Sight");
		go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("AlwaysCook\\Heavy.fbx", set), mat));
		WeaponModel->GetTransform()->SetLocalPosition(glm::vec3(1.2, -2, 3));//z,y,x
		WeaponModel->GetTransform()->SetScale(glm::vec3(0.4f));
	}
	s->AddGameobjectToScene(go);
}

Weapon::~Weapon()
{}

void Weapon::InitComponent()
{
	if (WeaponRoot == nullptr)
	{
		WeaponRoot = GetOwner();
	}
}

void Weapon::Update(float delta)
{
	CurrentCoolDown -= delta;
}

void Weapon::SetCurrentSettings(WeaponSettings NewSettings)
{
	CurrentSettings = NewSettings;
}

void Weapon::PlayFireSound()
{
	switch (CurrentWeaponType)
	{
	default:
		break;
	case ShotGun:
		AudioEngine::PostEvent("Play_Shotgun", GetOwner());
		break;
	case Rifle:
		AudioEngine::PostEvent("Play_Rifle", GetOwner());
		break;
	case RailGun:
		AudioEngine::PostEvent("Play_Gauss", GetOwner());
		break;
	case AIRifle:
		AudioEngine::PostEvent("AI_Rifle_Fire", GetOwner());
		break;
	case Limit:
		break;
	}
}

bool Weapon::Fire()
{
	if (CurrentCoolDown > 0)
	{
		return false;
	}
	CurrentCoolDown = CurrentSettings.FireDelay;
	if (CurrentAmmoCount <= 0)
	{
		AudioEngine::PostEvent("EmptyClick", GetOwner());
		return false;
	}
	PerfManager::StartTimer("Weapon::Fire");
	CurrentAmmoCount--;
	PlayFireSound();

	//Create projectile!
	glm::vec3 Forward = WeaponRoot->GetTransform()->GetForward();
	glm::vec3 offset = glm::vec3(0);
	if (Player != nullptr)
	{
		Forward = CameraComponent::GetMainCamera()->GetForward();
	}
	else
	{
		Forward = AIForward;
	}

	glm::vec3 Position = offset + WeaponRoot->GetPosition() + Forward * 4;
#if WITH_EDITOR
	DebugDrawers::DrawDebugLine(Position, Position + Forward * 10, glm::vec3(1), false, 1);
#endif
	GameObject* newgo = GameObject::Instantiate(Position);
	newgo->GetTransform()->SetScale(glm::vec3(0.3f));
	ColliderComponent* cc = newgo->AttachComponent(new ColliderComponent());
	cc->IsTrigger = true;
	cc->SetCollisonShape(EShapeType::eSPHERE);
	RigidbodyComponent* rb = newgo->AttachComponent(new RigidbodyComponent());
	rb->SetGravity(false);
	rb->SetLinearVelocity(Forward*CurrentSettings.ProjectileSpeed);
	Projectile* Proj = newgo->AttachComponent(new Projectile(GetOwner()));
	Proj->SetDamage(CurrentSettings.DamagePerShot);
	//todo: speed this up!
	if (CurrentSettings.ShowProjectile)
	{
		newgo->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Models\\Sphere.obj"), Material::GetDefaultMaterial()));
	}
	GameObject::FinishGameObjectSpawn(newgo);
	OnFire();
	PerfManager::EndTimer("Weapon::Fire");
	return true;
}

void Weapon::SetState(bool state)
{
	if (WeaponModel != nullptr && WeaponModel->GetMeshRenderer() != nullptr)
	{
		WeaponModel->GetMeshRenderer()->SetVisiblity(state);
	}
}

void Weapon::AddAmmo(int amt)
{
	CurrentAmmoCount += amt;
	CurrentAmmoCount = glm::clamp(CurrentAmmoCount, 0, CurrentSettings.MaxAmmoCount);
}

