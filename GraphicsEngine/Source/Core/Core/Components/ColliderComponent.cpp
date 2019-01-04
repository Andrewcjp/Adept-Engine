#include "ColliderComponent.h"
#include "CompoenentRegistry.h"
#include "Core/Assets/Archive.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Engine.h"
#include "Core/GameObject.h"
#include "Core/Utils/DebugDrawers.h"
#include "Physics/PhysicsEngine.h"
#include "Physics/SimTD/TDRigidBody.h"
#include "RigidbodyComponent.h"

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

void ColliderComponent::SetEnabled(bool State)
{
	Enabled = State;
	if (CurrentCollider)
	{
		CurrentCollider->SetEnabled(Enabled);
	}
}

void ColliderComponent::InitComponent()
{}

void ColliderComponent::BeginPlay()
{}

void ColliderComponent::Update(float)
{
	RenderShape();
}

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

#if WITH_EDITOR
void ColliderComponent::EditorUpdate()
{
	RenderShape();
}
#endif

void ColliderComponent::RenderShape()
{
	if (PhysicsEngine::GetCurrentMode() == EPhysicsDebugMode::ShowShapes || PhysicsEngine::GetCurrentMode() == EPhysicsDebugMode::All)
	{
		glm::vec3 colour = glm::vec3(1);
		if (IsTrigger)
		{
			colour = glm::vec3(0, 1, 0);
		}
		if (!Enabled)
		{
			colour = glm::vec3(0.3f);
		}
		switch (CollisionShapeType)
		{
			//todo: transition this to use a wire frame shader
		case EShapeType::eCAPSULE:
#if 0
			DebugDrawers::DrawDebugCapsule(GetOwner()->GetPosition() + LocalOffset, Height, Radius, GetOwner()->GetRotation(), colour);
			break;
#endif
		case EShapeType::eSPHERE:
			DebugDrawers::DrawDebugSphere(GetOwner()->GetPosition() + LocalOffset, Radius, colour);
			break;
		case EShapeType::eTRIANGLEMESH:
			return;
			for (int i = 0; i < Points.size(); i++)
			{
				for (int x = 0; x < 3; x++)
				{
					const int next = (x + 1) % 3;
					DebugDrawers::DrawDebugLine(Points[i][x], Points[i][next], colour, false, 0.0f);
				}
			}
			break;
		case EShapeType::eBOX:

			break;
		}
	}
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
#if USE_PHYSX
		LoadMesh();
#endif
		box->Scale = GetOwner()->GetTransform()->GetScale();
		return box;
	}
	}
	return nullptr;
}

Collider * ColliderComponent::GetCollider()
{
	if (CurrentCollider == nullptr)
	{
		CurrentCollider = new Collider();
		CurrentCollider->ComponentOwner = this;
	}
	CurrentCollider->Shapes.push_back(GetColliderShape());
	CurrentCollider->IsTrigger = IsTrigger;
	return CurrentCollider;
}

void ColliderComponent::LoadMesh()
{
	MeshLoader::FMeshLoadingSettings set;
	std::vector<OGLVertex> v;
	std::vector<int> inds;
	set.GenerateIndexed = true;
	MeshLoader::LoadMeshFromFile_Direct(MeshName, set, v, inds);
	Points.clear();
	for (int i = 0; i < inds.size(); i += 3)
	{
		glm::vec3* data = new glm::vec3[3];
		data[0] = v[inds[i]].m_position;
		data[1] = v[inds[i + 1]].m_position;
		data[2] = v[inds[i + 2]].m_position;
		Points.push_back(data);
	}
}

void ColliderComponent::SceneInitComponent()
{
	RigidComp = GetOwner()->GetComponent<RigidbodyComponent>();
	if ((GetOwner() != nullptr && RigidComp == nullptr) || GetOwner() == nullptr)
	{
		//A game object without a rigid body should be setup as a static rigid body
		Actor = new RigidBody(EBodyType::RigidStatic, GetOwner()->GetPosition());
		std::vector<ColliderComponent*> colliders = GetOwner()->GetAllComponentsOfType<ColliderComponent>();
		for (ColliderComponent* cc : colliders)
		{
			Actor->AttachCollider(cc->GetCollider());
		}
		Actor->SetGravity(false);
		Actor->InitBody();
		Actor->SetOwnerComponent(this);
	}
}

void ColliderComponent::MovePhysicsBody(glm::vec3 newpos, glm::quat newrot)
{
	if (Actor != nullptr)
	{
		Actor->SetPositionAndRotation(newpos, newrot);
		//Log::LogMessage(glm::to_string(newrot));
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
void ColliderComponent::GetInspectorProps(std::vector<InspectorProperyGroup>& props)
{
	InspectorProperyGroup group = Inspector::CreatePropertyGroup("Collider Component");
	group.SubProps.push_back(Inspector::CreateProperty("Type", EditValueType::Int, &CollisionShapeType, true));
	if (CollisionShapeType == EShapeType::eBOX)
	{
		group.SubProps.push_back(Inspector::CreateProperty("Extents", EditValueType::Vector, &BoxExtents));
	}
	else if (CollisionShapeType == EShapeType::eSPHERE)
	{
		group.SubProps.push_back(Inspector::CreateProperty("Radius", EditValueType::Float, &Radius));
	}
	else if (CollisionShapeType == EShapeType::ePLANE)
	{
		group.SubProps.push_back(Inspector::CreateProperty("Scale", EditValueType::Vector, &BoxExtents));
	}
	props.push_back(group);
}
#endif