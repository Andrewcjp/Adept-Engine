#include "stdafx.h"
#include "RigidbodyComponent.h"
#include "Core/GameObject.h"
#include "Core/Engine.h"
#include "Physics/PhysicsEngine.h"
#include "CompoenentRegistry.h"
#include "Core/Assets/Scene.h"
#include "ColliderComponent.h"
RigidbodyComponent::RigidbodyComponent()
{
	DoesFixedUpdate = true;
	TypeID = CompoenentRegistry::BaseComponentTypes::RigidComp;
}

RigidbodyComponent::~RigidbodyComponent()
{}

void RigidbodyComponent::BeginPlay()
{}

void RigidbodyComponent::Update(float delta)
{}

void RigidbodyComponent::OnTransformUpdate()
{
	if (actor != nullptr)
	{
		//	actor->SetPositionAndRotation(GetOwner()->GetTransform()->GetPos(), GetOwner()->GetTransform()->GetQuatRot());
	}
}

void RigidbodyComponent::FixedUpdate(float delta)
{
	if (actor != nullptr)
	{		
		GetOwner()->MoveComponent(actor->GetPosition(), GetOwner()->GetTransform()->GetQuatRot()/*actor->GetRotation()*/, false);
	}
}

void RigidbodyComponent::MovePhysicsBody(glm::vec3 newpos, glm::quat newrot)
{
	if (actor != nullptr)
	{
		actor->SetPositionAndRotation(newpos, newrot);
		//Log::LogMessage(glm::to_string(newrot));
	}
}

void RigidbodyComponent::SceneInitComponent()
{
	if (Engine::PhysEngine != nullptr && GetOwnerScene() && !GetOwnerScene()->IsEditorScene())
	{
		actor = new RigidBody(EBodyType::RigidDynamic, *GetOwner()->GetTransform());
		std::vector<ColliderComponent*> colliders = GetOwner()->GetAllComponentsOfType<ColliderComponent>();
		Collider* tempcol = new Collider();
		for (ColliderComponent* cc : colliders)
		{
			tempcol->Shapes.push_back(cc->GetColliderShape());
		}
		actor->AttachCollider(tempcol);
		actor->SetLockFlags(LockData);
		actor->InitBody();
	}
}

void RigidbodyComponent::SetLinearVelocity(glm::vec3 velocity)
{
	if (actor != nullptr)
	{
		actor->SetLinearVelocity(velocity);
	}
}

glm::vec3 RigidbodyComponent::GetVelocity()
{
	if (actor != nullptr)
	{
		return actor->GetLinearVelocity();
	}
	return glm::vec3();
}
#if WITH_EDITOR
void RigidbodyComponent::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props)
{
	Inspector::InspectorProperyGroup group = Inspector::CreatePropertyGroup("RigidBody Component");
	group.SubProps.push_back(Inspector::CreateProperty("Mass", EditValueType::Float, &mass));
	props.push_back(group);
}
#endif
void RigidbodyComponent::ProcessSerialArchive(Archive * A)
{
	Component::ProcessSerialArchive(A);
}

void RigidbodyComponent::SetLockFlags(BodyInstanceData data)
{
	LockData = data;
	if (actor)
	{
		actor->SetLockFlags(LockData);
	}
}

BodyInstanceData RigidbodyComponent::GetLockFlags()
{
	return LockData;
}
