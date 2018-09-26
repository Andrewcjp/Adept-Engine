#include "stdafx.h"
#include "RigidbodyComponent.h"
#include "Core/GameObject.h"
#include "Core/Engine.h"
#include "Physics/PhysicsEngine.h"
#include "CompoenentRegistry.h"
#include "Core/Assets/Scene.h"
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

void RigidbodyComponent::FixedUpdate(float delta)
{
	if (actor != nullptr)
	{
		GetOwner()->GetTransform()->SetPos(actor->GetPosition());
		GetOwner()->GetTransform()->SetQrot(actor->GetRotation());
	}
}

void RigidbodyComponent::SceneInitComponent()
{
	if (Engine::PhysEngine != nullptr && GetOwnerScene() && !GetOwnerScene()->IsEditorScene())
	{
		actor = Engine::PhysEngine->CreatePrimitiveRigidBody(GetOwner()->GetTransform()->GetPos(), glm::vec3(0), 1);
	}
}

void RigidbodyComponent::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props)
{
	Inspector::InspectorProperyGroup group = Inspector::CreatePropertyGroup("RigidBody Component");
	//group.SubProps.push_back(Inspector::CreateProperty("test", Inspector::Float, nullptr));
	props.push_back(group);
}

void RigidbodyComponent::ProcessSerialArchive(Archive * A)
{
	Component::ProcessSerialArchive(A);
}


