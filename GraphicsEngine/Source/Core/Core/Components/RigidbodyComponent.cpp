
#include "RigidbodyComponent.h"
#include "Core/GameObject.h"
#include "Core/Engine.h"
#include "Physics/PhysicsEngine.h"

#include "Core/Assets/Scene.h"
#include "ColliderComponent.h"
#include "../Utils/MathUtils.h"
#include "../Assets/Archive.h"
RigidbodyComponent::RigidbodyComponent()
{
	DoesFixedUpdate = true;
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
		BodyData.Gravity = active;
	}
}

void RigidbodyComponent::FixedUpdate(float delta)
{
	if (actor != nullptr)
	{
		//todo: fix transform feedback issue in PHYSX
		CheckNAN(actor->GetPosition());
		CheckNAN(actor->GetRotation());
		GetOwner()->MoveComponent(actor->GetPosition(), IsKineimatic ? GetOwner()->GetTransform()->GetQuatRot() : actor->GetRotation(), false);
	}
}

void RigidbodyComponent::MovePhysicsBody(glm::vec3 newpos, glm::quat newrot)
{
	if (actor != nullptr)
	{
//		actor->SetPositionAndRotation(newpos, newrot);
		//Log::LogMessage(glm::to_string(newrot));
	}
}

void RigidbodyComponent::SceneInitComponent()
{
	if (Engine::PhysEngine != nullptr && GetOwnerScene() && !GetOwnerScene()->IsEditorScene())
	{
		//actor = new RigidBody(EBodyType::RigidDynamic, *GetOwner()->GetTransform());
		//std::vector<ColliderComponent*> colliders = GetOwner()->GetAllComponentsOfType<ColliderComponent>();
		//for (ColliderComponent* cc : colliders)
		//{
		//	cc->TransferToRigidbody();
		//	actor->AttachCollider(cc->GetCollider());
		//}
		//actor->SetBodyData(BodyData);
		//actor->SetOwnerComponent(this);
		////actor->InitBody();
		//actor->SetLinearVelocity(InitalVelocity);
		//actor->SetGravity(BodyData.Gravity);
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
//	InspectorProperyGroup group = Inspector::CreatePropertyGroup("RigidBody Component");
////	group.SubProps.push_back(Inspector::CreateProperty("Mass", EditValueType::Float, &mass));
//	props.push_back(group);
}
#endif
void RigidbodyComponent::ProcessSerialArchive(Archive * A)
{
	Component::ProcessSerialArchive(A);
	ArchiveProp(BodyData.IsKinematic);
}

void RigidbodyComponent::SetBodyData(BodyInstanceData data)
{
	BodyData = data;
	if (actor)
	{
		actor->SetBodyData(BodyData);
	}
}

BodyInstanceData& RigidbodyComponent::GetBodyData()
{
	if (actor != nullptr)
	{
		return actor->GetBodyData();
	}
	return BodyData;
}
