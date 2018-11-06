
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
{
	SafeDelete(actor);
}

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
RigidBody * RigidbodyComponent::GetActor()
{
	return actor;
}
void RigidbodyComponent::SetGravity(bool active)
{
	if (actor != nullptr)
	{
		actor->SetGravity(active);
	}
	else
	{
		LockData.Gravity = active;
	}
}

void RigidbodyComponent::FixedUpdate(float delta)
{
	if (actor != nullptr)
	{	
		//todo: fix transform feedback issue in PHYSX
#if 0//PHYSX_ENABLED
		GetOwner()->MoveComponent(actor->GetPosition(), GetOwner()->GetTransform()->GetQuatRot()/*actor->GetRotation()*/, false);
#else
		GetOwner()->MoveComponent(actor->GetPosition(), actor->GetRotation(), false);
#endif
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
			cc->TransferToRigidbody();
			tempcol->Shapes.push_back(cc->GetColliderShape());
		}
		actor->AttachCollider(tempcol);
		actor->SetBodyData(LockData);
		actor->SetOwnerComponent(this);
		actor->InitBody();
		actor->SetLinearVelocity(InitalVelocity);
		actor->SetGravity(LockData.Gravity);
	}
}

void RigidbodyComponent::SetLinearVelocity(glm::vec3 velocity)
{
	if (actor != nullptr)
	{
		actor->SetLinearVelocity(velocity);
	}
	else
	{
		InitalVelocity = velocity;
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
void RigidbodyComponent::GetInspectorProps(std::vector<InspectorProperyGroup>& props)
{
	InspectorProperyGroup group = Inspector::CreatePropertyGroup("RigidBody Component");
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
		actor->SetBodyData(LockData);
	}
}

BodyInstanceData RigidbodyComponent::GetLockFlags()
{
	return LockData;
}
