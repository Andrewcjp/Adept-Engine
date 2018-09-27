//#include "stdafx.h"
#include "ColliderComponent.h"
#include "Core/Engine.h"
#include "Physics/PhysicsEngine.h"
#include "CompoenentRegistry.h"
#include "Core/Assets/Archive.h"
ColliderComponent::ColliderComponent()
{
	TypeID = CompoenentRegistry::BaseComponentTypes::ColliderComp;
}


ColliderComponent::~ColliderComponent()
{}

void ColliderComponent::InitComponent()
{

}

void ColliderComponent::BeginPlay()
{}

void ColliderComponent::Update(float)
{}

void ColliderComponent::ProcessSerialArchive(Archive * A)
{
	Component::ProcessSerialArchive(A);
	ArchiveProp_Enum(CollisionShapeType, EShapeType::Type);
	if (CollisionShapeType == EShapeType::eBOX)
	{
		ArchiveProp(BoxExtents);
	}
	else if (CollisionShapeType == EShapeType::eSPHERE)
	{
		ArchiveProp(Radius);
	}
}

EShapeType::Type ColliderComponent::GetCollisonShape()
{
	return CollisionShapeType;
}

void ColliderComponent::SetCollisonShape(EShapeType::Type newtype)
{
	CollisionShapeType = newtype;
}

ShapeElem * ColliderComponent::GetColliderShape()
{
	switch (CollisionShapeType)
	{
	case EShapeType::eBOX:
	{
		BoxElem* box = new BoxElem();
		box->Extents = BoxExtents;
		return box;
	}
	case EShapeType::eSPHERE:
	{
		SphereElem* box = new SphereElem();
		box->raduis = Radius;
		return box;
	}
	}
	return nullptr;
}

void ColliderComponent::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props)
{
	Inspector::InspectorProperyGroup group = Inspector::CreatePropertyGroup("Collider Component");
	group.SubProps.push_back(Inspector::CreateProperty("Type", Inspector::Int, &CollisionShapeType, true));
	if (CollisionShapeType == EShapeType::eBOX)
	{
		group.SubProps.push_back(Inspector::CreateProperty("Extents", Inspector::Vector, &BoxExtents));
	}
	else if (CollisionShapeType == EShapeType::eSPHERE)
	{
		group.SubProps.push_back(Inspector::CreateProperty("Raduis", Inspector::Float, &Radius));
	}
	props.push_back(group);
}
