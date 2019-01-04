#pragma once
#include "Core/Components/Component.h"
namespace PickupType
{
	enum Type
	{
		Rifle_Ammo,
		Gauss_Ammo,
		Shotgun_Ammo,
		Health,
		Limit
	};
};
class Pickup :public Component
{
public:
	Pickup();
	~Pickup();
	virtual void OnTrigger(CollisonData data) override;
	static void SpawnPickup(glm::vec3 position, PickupType::Type type, int value);
	virtual void SceneInitComponent() override;
private:
	int Value = 1;
	PickupType::Type PickupType = PickupType::Limit;
};

