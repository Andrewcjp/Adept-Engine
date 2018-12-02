#include "Source/TestGame/TestGamePCH.h"
#include "Railgun.h"
#include "TestPlayer.h"
#include "Weapon.h"
#include "Core/Components/RigidbodyComponent.h"
#include "Physics/SimTD/TDRigidBody.h"

Railgun::Railgun(Scene* scene, TestPlayer* cameraobj) :Weapon(Weapon::RailGun, scene, cameraobj)
{}

Railgun::~Railgun()
{}

void Railgun::Update(float delta)
{
	Weapon::Update(delta);
	if (timer > 0.0f)
	{
		timer -= delta;
		if (Player != nullptr)
		{
			Player->ExtraVel = vector * 10;
		}
	}
	else
	{
		if (Player != nullptr)
		{
			Player->ExtraVel = glm::vec3(0);
		}
	}
}

void Railgun::OnFire()
{
	if (Player != nullptr)
	{
		timer = 0.2f;
		vector = -Player->CameraObject->GetTransform()->GetForward();
		float thres = 0.02f;
		if (!Player->GetIsGrounded())
		{
			vector.xz *= glm::vec2(3);
			thres = 0.16f;
		}
		vector.y = glm::clamp(vector.y, -thres, thres);
	}
}
//small jump when grounded
//force over time
//push back small