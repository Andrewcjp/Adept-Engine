#include "Weapon.h"
#include "Projectile.h"
#include "Audio/AudioEngine.h"
#include "TestPlayer.h"

Weapon::Weapon(Weapon::WeaponType T, Scene* scene, TestPlayer* player)
{
	if (T == Rifle)
	{
		CurrentSettings.FireDelay = 0.135f;
		CurrentSettings.DamagePerShot = 20;
		CurrentSettings.MaxAmmoCount = 250;
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
	}
	Player = player;
	CurrentWeaponType = T;
	CreateModel(scene, player->CameraObject);
}

void Weapon::CreateModel(Scene* s, GameObject* cameraobj)
{
	GameObject* go = new GameObject("Gun Test");
	Material* mat = Material::GetDefaultMaterial();
	go->GetTransform()->SetPos(glm::vec3(0, 2, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	WeaponModel = go;
	WeaponModel->GetTransform()->SetParent(cameraobj->GetTransform());
	if (CurrentWeaponType == WeaponType::Rifle)
	{
		mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("Weapons\\Rifle\\Textures\\Variation 06\\Rifle_06_Albedo.png"));
		MeshLoader::FMeshLoadingSettings set;
		set.FlipUVs = true;
		go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Weapons\\Rifle\\Rifle.fbx", set), mat));
		WeaponModel->GetTransform()->SetLocalPosition(glm::vec3(1.2, -1, 2.5));
		WeaponModel->GetTransform()->SetScale(glm::vec3(0.5f));
	}
	else if (CurrentWeaponType == WeaponType::ShotGun)
	{

	}
	else if (CurrentWeaponType == WeaponType::RailGun)
	{
		mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("Weapons\\Heavy\\Textures\\Variation 08\\Heavy_08_Albedo.png"));
		MeshLoader::FMeshLoadingSettings set;
		set.FlipUVs = true;
		go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Weapons\\Heavy\\Heavy.fbx", set), mat));
		WeaponModel->GetTransform()->SetLocalPosition(glm::vec3(1.2, -2, 3));//z,y,x
		WeaponModel->GetTransform()->SetScale(glm::vec3(0.4f));
	}

	s->AddGameobjectToScene(go);
}

Weapon::~Weapon()
{}

void Weapon::InitComponent()
{

}

void Weapon::Update(float delta)
{
	CurrentCoolDown -= delta;
}

void Weapon::SetCurrentSettings(WeaponSettings NewSettings)
{
	CurrentSettings = NewSettings;
}

void Weapon::Fire()
{
	if (CurrentCoolDown > 0)
	{
		return;
	}
	AudioEngine::PostEvent("Play_Shotgun", GetOwner());
	//Create projectile!
	const glm::vec3 Forward = CameraComponent::GetMainCamera()->GetForward();
	TestPlayer* Player = GetOwner()->GetComponent<TestPlayer>();
	glm::vec3 Position = Player->CameraObject->GetPosition() + Forward * 4;
	GameObject* newgo = GameObject::Instantiate(Position);
	newgo->GetTransform()->SetScale(glm::vec3(0.3f));
	ColliderComponent* cc = newgo->AttachComponent(new ColliderComponent());
	RigidbodyComponent* rb = newgo->AttachComponent(new RigidbodyComponent());
	BodyInstanceData t;
	t.IsTrigger = true;
	rb->SetLockFlags(t);
	rb->SetGravity(false);
	rb->SetLinearVelocity(Forward*ProjectileSpeed);
	Projectile* Proj = newgo->AttachComponent(new Projectile());
	Proj->SetDamage(CurrentSettings.DamagePerShot);
	newgo->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Models\\Sphere.obj"), Material::GetDefaultMaterial()));
	GameObject::FinishGameObjectSpawn(newgo);
	
	CurrentCoolDown = CurrentSettings.FireDelay;
	OnFire();
}

void Weapon::SetState(bool state)
{
	if (WeaponModel->GetMeshRenderer() != nullptr)
	{
		WeaponModel->GetMeshRenderer()->SetVisiblity(state);
	}
}

