#include "Weapon.h"
#include "EngineHeader.h"
#include "Core/Components/Core_Components_inc.h"
#include "Projectile.h"
#include "Editor/EditorWindow.h"
#include "Audio/AudioEngine.h"
Weapon::Weapon()
{}


Weapon::~Weapon()
{}

void Weapon::InitComponent()
{

}

void Weapon::Update(float delta)
{
	if (EditorWindow::GetInstance()->IsEditorEjected())
	{
		return;
	}
	CurrentCoolDown -= delta;
	if (Input::GetMouseButtonDown(0))
	{
		Fire();
	}
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
	glm::vec3 Position = GetOwner()->GetPosition() + Forward * 3;
	GameObject* newgo = GameObject::Instantiate(Position);
	newgo->GetTransform()->SetScale(glm::vec3(0.3f));
	newgo->AttachComponent(new ColliderComponent());
	RigidbodyComponent* rb = newgo->AttachComponent(new RigidbodyComponent());
	rb->SetGravity(false);
	rb->SetLinearVelocity(Forward*ProjectileSpeed);
	Projectile* Proj = newgo->AttachComponent(new Projectile());
	Proj->SetDamage(10);
	newgo->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Sphere.obj"), Material::GetDefaultMaterial()));
	GameObject::FinishGameObjectSpawn(newgo);
	CurrentCoolDown = CurrentSettings.FireDelay;
}
