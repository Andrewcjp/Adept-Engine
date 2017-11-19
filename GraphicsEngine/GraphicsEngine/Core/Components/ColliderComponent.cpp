//#include "stdafx.h"
#include "ColliderComponent.h"
#include "../Engine.h"
#include "../Physics/PhysxEngine.h"

ColliderComponent::ColliderComponent()
{
}


ColliderComponent::~ColliderComponent()
{
}

void ColliderComponent::InitComponent()
{
	//CreateShape();
}

void ColliderComponent::BeginPlay()
{
}

void ColliderComponent::Update(float delta)
{
}

void ColliderComponent::CreateShape()
{
	if (CollisionShape != nullptr && InitalisedCollisionShapeType != CollisionShapeType)
	{
		CollisionShape->release();
	}
	switch (CollisionShapeType)
	{
	case Box:
		CollisionShape = Engine::PPhysxEngine->GetGPhysics()->createShape(PxBoxGeometry(BoxHalfExtent, BoxHalfExtent, BoxHalfExtent),
			*(Engine::PPhysxEngine->GetDefaultMaterial()));
		break;
	case Sphere:
		CollisionShape = Engine::PPhysxEngine->GetGPhysics()->createShape(PxSphereGeometry(Radius),
			*(Engine::PPhysxEngine->GetDefaultMaterial()));
		break;
	}
	InitalisedCollisionShapeType = CollisionShapeType;
}

ColliderComponent::ShapeType ColliderComponent::GetCollisonShape()
{
	return CollisionShapeType;
}

void ColliderComponent::SetCollisonShape(ShapeType newtype)
{
	CollisionShapeType = newtype;
}

physx::PxShape * ColliderComponent::Internal_GetPhysxShape()
{
	if (CollisionShape == nullptr)
	{
		CreateShape();
	}
	return CollisionShape;
}
