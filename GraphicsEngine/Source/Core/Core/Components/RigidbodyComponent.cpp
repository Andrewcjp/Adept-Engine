#include "stdafx.h"
#include "RigidbodyComponent.h"
#include "Core/GameObject.h"
#include "Core/Engine.h"
#include "Physics/PhysicsEngine.h"
RigidbodyComponent::RigidbodyComponent()
{
	DoesFixedUpdate = true;
}


RigidbodyComponent::~RigidbodyComponent()
{
}

void RigidbodyComponent::BeginPlay()
{
}

void RigidbodyComponent::Update(float delta)
{
}

void RigidbodyComponent::FixedUpdate(float delta)
{
	if (actor != nullptr)
	{
		GetOwner()->GetTransform()->SetPos(actor->GetPosition());
		GetOwner()->GetTransform()->SetQrot(actor->GetRotation());
	}
}

void RigidbodyComponent::InitComponent()
{
	if (Engine::PhysEngine != nullptr)
	{
		actor = Engine::PhysEngine->CreatePrimitiveRigidBody(GetOwner()->GetTransform()->GetPos(), glm::vec3(0), 1);
	}
}

