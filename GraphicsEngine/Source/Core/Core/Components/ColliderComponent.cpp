//#include "stdafx.h"
#include "ColliderComponent.h"
#include "Core/Engine.h"
#include "Physics/PhysicsEngine.h"

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

void ColliderComponent::Update(float )
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
		CollisionShape = Engine::PhysEngine->GetGPhysics()->createShape(physx::PxBoxGeometry(BoxHalfExtent, BoxHalfExtent, BoxHalfExtent),
			*(Engine::PhysEngine->GetDefaultMaterial()));
		break;
	case Sphere:
		CollisionShape = Engine::PhysEngine->GetGPhysics()->createShape(physx::PxSphereGeometry(Radius),
			*(Engine::PhysEngine->GetDefaultMaterial()));
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
