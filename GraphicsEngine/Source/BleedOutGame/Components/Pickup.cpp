#include "Pickup.h"
#include "Core/Components/ColliderComponent.h"
#include "Core/Components/MeshRendererComponent.h"
#include "Health.h"
#include "BleedOutPlayer.h"
#include "WeaponManager.h"

Pickup::Pickup()
{}


Pickup::~Pickup()
{}

void Pickup::OnTrigger(CollisonData data)
{
	BleedOutPlayer* player = data.OtherCollider->GetGameObject()->GetComponent<BleedOutPlayer>();
	if (player != nullptr)
	{
		switch (PickupType)
		{
		case PickupType::Rifle_Ammo:
		{
			player->Manager->AddAmmo(Weapon::Rifle, Value);
		}
		break;
		case PickupType::Gauss_Ammo:
		{
			player->Manager->AddAmmo(Weapon::RailGun, Value);
		}
		break;
		case PickupType::Shotgun_Ammo:
		{
			player->Manager->AddAmmo(Weapon::ShotGun, Value);
		}
		break;
		case PickupType::Health:
		{
			Health* h = player->GetOwner()->GetComponent<Health>();
			h->AddHealth((float)Value);
			break;
		}
		default:
			break;
		}
		GetOwner()->Destory();
	}	
}

void Pickup::SpawnPickup(glm::vec3 position, PickupType::Type type, int value)
{

	GameObject* obj = new GameObject("Pickup");
	obj->SetPosition(position);
	ColliderComponent* cc = obj->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eSPHERE);
	cc->Radius = 2.0f;
	cc->IsTrigger = true;
	//cc->SetEnabled(false);
	switch (type)
	{
	case PickupType::Gauss_Ammo:
	case PickupType::Rifle_Ammo:
	{
		Material* mat = Material::CreateDefaultMaterialInstance();
		mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\ammoc03.jpg"));
		MeshLoader::FMeshLoadingSettings set;
		//set.FlipUVs = true;
		obj->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("AlwaysCook\\CubeUv.obj", set), mat));
	}
	break;
	case PickupType::Health:
	{
		Material* mat = Material::CreateDefaultMaterialInstance();
		mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
		obj->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("models\\HealthPickup.obj"), mat));
	}
	break;
	default:
		break;
	}
	Pickup* pick = obj->AttachComponent(new Pickup());
	pick->PickupType = type;
	pick->Value = value;
	GameObject::FinishGameObjectSpawn(obj);
}

void Pickup::SceneInitComponent()
{

}


