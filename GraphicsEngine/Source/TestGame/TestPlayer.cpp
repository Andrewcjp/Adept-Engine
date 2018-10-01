#include "TestPlayer.h"
#include "EngineHeader.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Components/Core_Components_inc.h"
TestPlayer::TestPlayer()
{

}


TestPlayer::~TestPlayer()
{}

void TestPlayer::InitComponent()
{

}

void TestPlayer::BeginPlay()
{
	RB = GetOwner()->GetComponent<RigidbodyComponent>();
}

void TestPlayer::Update(float delta)
{
	glm::vec3 TargetVel = RB->GetVelocity();
	TargetVel.z = 0.0f;
	TargetVel.x = 0.0f;
	if (Input::GetKey('a'))
	{
		TargetVel += GetOwner()->GetTransform()->GetRight()* Speed;
	}
	if (Input::GetKey('d'))
	{
		TargetVel -= GetOwner()->GetTransform()->GetRight()*Speed;
	}
	if (Input::GetKeyDown(KeyCode::SPACE))
	{
		TargetVel -= GetOwner()->GetTransform()->GetUp() * jumpHeight;
	}
	if (RB != nullptr)
	{
		RB->SetLinearVelocity(TargetVel);
	}
	if (CameraComponent::GetMainCamera() != nullptr)
	{
		glm::vec3 Pos = GetOwner()->GetTransform()->GetPos();
		Pos.z = 0;
		CameraComponent::GetMainCamera()->SetPos(Pos);
	}
}

