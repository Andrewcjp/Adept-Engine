//#include "stdafx.h"
#include "ColliderComponent.h"
#include "Core/Engine.h"
#include "Physics/PhysicsEngine.h"
#include "CompoenentRegistry.h"
#include "Core/Assets/Archive.h"
#include "Core/GameObject.h"
#include "RigidbodyComponent.h"
#include "Core/Assets/AssetManager.h"
ColliderComponent::ColliderComponent()
{
	TypeID = CompoenentRegistry::BaseComponentTypes::ColliderComp;
}


ColliderComponent::~ColliderComponent()
{
	if (Actor != nullptr)
	{
		Actor->SetOwnerComponent(nullptr);
	}
	SafeDelete(Actor);
}

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
	else if (CollisionShapeType == EShapeType::eTRIANGLEMESH || CollisionShapeType == EShapeType::eCONVEXMESH)
	{
		ArchiveProp(MeshName);
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
	case EShapeType::ePLANE:
	{
		PlaneElm* box = new PlaneElm();
		box->scale = BoxExtents;
		return box;
	}
	case EShapeType::eCAPSULE:
	{
		CapsuleElm* cap = new CapsuleElm();
		cap->raduis = Radius;
		cap->height = Height;
		return cap;
	}
	case EShapeType::eCONVEXMESH:
	{
		if (MeshName.length() == 0)
		{
			return nullptr;
		}
		ConvexMeshElm* box = new ConvexMeshElm();
		box->MeshAssetName = MeshName;
		box->Scale = GetOwner()->GetTransform()->GetScale();
		return box;
	}
	case EShapeType::eTRIANGLEMESH:
	{
		if (MeshName.length() == 0)
		{
			return nullptr;
		}
		TriMeshElm* box = new TriMeshElm();
		box->MeshAssetName = MeshName;
		box->Scale = GetOwner()->GetTransform()->GetScale();
		return box;
	}
	}
	return nullptr;
}

void ColliderComponent::SceneInitComponent()
{
	if ((GetOwner() != nullptr && GetOwner()->GetComponent<RigidbodyComponent>() == nullptr) || GetOwner() == nullptr)
	{
		//A gameobject without a rigidbody should be setup as a static rigidbody
		Actor = new RigidBody(EBodyType::RigidStatic, GetOwner()->GetPosition());
		std::vector<ColliderComponent*> colliders = GetOwner()->GetAllComponentsOfType<ColliderComponent>();
		Collider* tempcol = new Collider();
		for (ColliderComponent* cc : colliders)
		{
			tempcol->Shapes.push_back(cc->GetColliderShape());
		}
		Actor->AttachCollider(tempcol);
		Actor->InitBody();
		Actor->SetOwnerComponent(this);
	}
}
void ColliderComponent::TransferToRigidbody()
{
	SafeDelete(Actor);
}

void ColliderComponent::SetTriangleMeshAssetName(std::string name)
{
	MeshName = AssetManager::GetContentPath() + name;
}
#if WITH_EDITOR
void ColliderComponent::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props)
{
	Inspector::InspectorProperyGroup group = Inspector::CreatePropertyGroup("Collider Component");
	group.SubProps.push_back(Inspector::CreateProperty("Type", EditValueType::Int, &CollisionShapeType, true));
	if (CollisionShapeType == EShapeType::eBOX)
	{
		group.SubProps.push_back(Inspector::CreateProperty("Extents", EditValueType::Vector, &BoxExtents));
	}
	else if (CollisionShapeType == EShapeType::eSPHERE)
	{
		group.SubProps.push_back(Inspector::CreateProperty("Raduis", EditValueType::Float, &Radius));
	}
	else if (CollisionShapeType == EShapeType::ePLANE)
	{
		group.SubProps.push_back(Inspector::CreateProperty("Scale", EditValueType::Vector, &BoxExtents));
	}
	props.push_back(group);
}
#endif