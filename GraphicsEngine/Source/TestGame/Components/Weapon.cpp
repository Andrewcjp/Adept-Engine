#include "Weapon.h"
#include "Projectile.h"
#include "Audio/AudioEngine.h"
#include "TestPlayer.h"
Weapon::Weapon()
{}

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
	newgo->AttachComponent(new ColliderComponent());
	RigidbodyComponent* rb = newgo->AttachComponent(new RigidbodyComponent());
	rb->SetGravity(false);
	rb->SetLinearVelocity(Forward*ProjectileSpeed);
	Projectile* Proj = newgo->AttachComponent(new Projectile());
	Proj->SetDamage(10);
	newgo->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Models\\Sphere.obj"), Material::GetDefaultMaterial()));
	GameObject::FinishGameObjectSpawn(newgo);
	CurrentCoolDown = CurrentSettings.FireDelay;
}

void Weapon::SetWeaponModel(GameObject * Model, GameObject* cameraobj)
{
	WeaponModel = Model;
	WeaponModel->GetTransform()->SetParent(cameraobj->GetTransform());
	WeaponModel->GetTransform()->SetLocalPosition(glm::vec3(1.2, -1, 2.5));
	WeaponModel->GetTransform()->SetScale(glm::vec3(0.5f));
	//WeaponModel->SetPosition(CurrentSettings.WeaponRelativePos);
}
