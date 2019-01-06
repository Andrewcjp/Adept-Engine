#if TDSIM_ENABLED
#include "TDRigidBody.h"
#include "TDRigidDynamic.h"
#include "TDPhysicsEngine.h"
#include "Shapes/TDSphere.h"
#include "Shapes/TDBox.h"
#include "Shapes/TDMeshShape.h"
#include "Core/Utils/DebugDrawers.h"
#include "Core/Components/ColliderComponent.h"
#include "Core/Platform/ConsoleVariable.h"

TDRigidBody::TDRigidBody(EBodyType::Type type, Transform T) :GenericRigidBody(type)
{
	m_transform = T;
}

TDRigidBody::~TDRigidBody()
{
	TDPhysicsEngine::GetScene()->RemoveActor(CommonActorPTr);
	CommonActorPTr = nullptr;
	SafeDelete(Actor);
	SafeDelete(StaticActor);
}

glm::vec3 TDRigidBody::GetPosition() const
{
	return Actor->GetTransfrom()->GetPos();
}

glm::quat TDRigidBody::GetRotation() const
{
	return Actor->GetTransfrom()->GetQuatRot();
}

void TDRigidBody::AddTorque(glm::vec3 torque)
{
	Actor->AddTorque(torque);
}

void TDRigidBody::AddForce(glm::vec3 force, EForceMode::Type Mode)
{
	Actor->AddForce(force, (Mode == EForceMode::AsForce) ? TDForceMode::AsAcceleration : TDForceMode::AsForce);
}

glm::vec3 TDRigidBody::GetLinearVelocity()const
{
	return Actor->GetLinearVelocity();
}

void TDRigidBody::AttachCollider(Collider * col)
{
	for (int i = 0; i < col->Shapes.size(); i++)
	{
		ShapeElem* Shape = col->Shapes[i];
		if (Shape == nullptr)
		{
			Log::LogMessage("Invalid physx Shape", Log::Severity::Error);
			continue;
		}
		TDShape* newShape = nullptr;
		switch (Shape->GetType())
		{
		case EShapeType::eBOX:
		{
			BoxElem* BoxShape = (BoxElem*)Shape;
			newShape = new TD::TDBox(BoxShape->Extents);
			break;
		}
		case EShapeType::eCAPSULE:
		{
			CapsuleElm* SphereShape = (CapsuleElm*)Shape;
			newShape = new TD::TDSphere();//todo: Capsule
			((TD::TDSphere*)newShape)->Radius = SphereShape->raduis;
			break;
		}
		case EShapeType::eSPHERE:
		{
			SphereElem* SphereShape = (SphereElem*)Shape;
			newShape = new TD::TDSphere();
			((TD::TDSphere*)newShape)->Radius = SphereShape->raduis;

			break;
		}
		case EShapeType::eTRIANGLEMESH:
		{
			TriMeshElm* SphereShape = (TriMeshElm*)Shape;
			newShape = new TD::TDMeshShape(GenerateTriangleMesh(SphereShape->MeshAssetName, glm::vec3(1)));
			break;
		}
		}
		if (col->IsTrigger)
		{
			newShape->GetFlags().SetFlagValue(TDShapeFlags::ESimulation, false);
			newShape->GetFlags().SetFlagValue(TDShapeFlags::ETrigger, true);
		}
		col->Shape = newShape;
		col->SetEnabled(col->ComponentOwner->IsEnabled());
		AttachedColliders.push_back(col);
		col->SetOwner(this);
		newShape->LocalPos = col->ComponentOwner->LocalOffset;
		//if (!col->IsTrigger)
		{
			shapes.push_back(newShape);
		}
		newShape->UserData = col;
	}
}

TD::TDMesh* TDRigidBody::GenerateTriangleMesh(std::string Filename, glm::vec3 scale)
{
	std::vector<OGLVertex> vertices;
	std::vector<int> indices;
	MeshLoader::FMeshLoadingSettings t;
	t.Scale = scale;
	MeshLoader::LoadMeshFromFile_Direct(Filename, t, vertices, indices);
	std::vector <glm::vec3> verts;
	std::vector <glm::vec3> normals;
	for (int i = 0; i < vertices.size(); i++)
	{
		verts.push_back(vertices[i].m_position);
		normals.push_back(vertices[i].m_normal);
	}
	TD::TDTriangleMeshDesc desc;

	desc.Points.Count = verts.size();
	desc.Points.Stride = sizeof(glm::vec3);
	desc.Points.DataPtr = verts.data();

	desc.Indices.Count = indices.size() /*/ 2*/;
	desc.Indices.Stride = sizeof(int);
	desc.Indices.DataPtr = indices.data();

	desc.HasPerVertexNormals = true;
	desc.Normals.Count = normals.size();
	desc.Normals.Stride = sizeof(glm::vec3);
	desc.Normals.DataPtr = normals.data();

	TD::TDMesh* newmesh = new TD::TDMesh(desc);
	newmesh->CookMesh();
#if 0
	std::vector<TDTriangle*> tris = newmesh->GetTriangles();
	for (int i = 0; i < tris.size(); i++)
	{
		const int sides = 3;
		for (int x = 0; x < sides; x++)
		{
			const int next = (x + 1) % sides;
			DebugDrawers::DrawDebugLine(tris[i]->Points[x], tris[i]->Points[next], glm::vec3(1), false, 100.0f);
		}
	}
#endif

	return newmesh;
}

void TDRigidBody::SetLinearVelocity(glm::vec3 velocity)
{
	if (Actor)
	{
		Actor->SetLinearVelocity(velocity);
	}
}

void TDRigidBody::InitBody()
{
	if (BodyType == EBodyType::RigidStatic)
	{
		StaticActor = new TD::TDRigidStatic();
		CommonActorPTr = StaticActor;
	}
	else
	{
		Actor = new TD::TDRigidDynamic();
		Actor->SetGravity(BodyData.Gravity);
		CommonActorPTr = Actor;
	}
	CommonActorPTr->GetTransfrom()->SetPos(m_transform.GetPos());
	for (TD::TDShape* s : shapes)
	{
		CommonActorPTr->AttachShape(s);
	}
	TDPhysicsEngine::GetScene()->AddToScene(CommonActorPTr);
	CommonActorPTr->UserData = this;
	UpdateBodyState();
}

void TDRigidBody::SetPositionAndRotation(glm::vec3 pos, glm::quat rot)
{
	if (CommonActorPTr)
	{
		CommonActorPTr->GetTransfrom()->SetPos(pos);
		CommonActorPTr->GetTransfrom()->SetQrot(rot);
	}
}

void TDRigidBody::SetGravity(bool state)
{
	if (Actor)
	{
		Actor->SetGravity(state);
	}
	else
	{
		BodyData.Gravity = state;
	}
}

TD::TDActor* TDRigidBody::GetActor()
{
	return CommonActorPTr;
}

float TDRigidBody::GetMass()
{
	return Actor->GetBodyMass();
}

void TDRigidBody::SetPhysicsMaterial(PhysicalMaterial * Mat)
{
	PhysicsMat = Mat;
	if (Mat == nullptr)
	{
		return;
	}
	SafeDelete(TDMaterial);
	TDMaterial = TDPhysicsEngine::CreatePhysicsMaterial(PhysicsMat);
}
static ConsoleVariable FullRot("frot", 0, ECVarType::LaunchOnly);
void TDRigidBody::UpdateBodyState()
{
	if (Actor != nullptr)
	{
		Actor->SetBodyMass(BodyData.Mass);
		SetPhysicsMaterial(BodyData.Mat);
		if (PhysicsMat != nullptr && TDMaterial != nullptr)
		{
			for (int i = 0; i < Actor->GetAttachedShapes().size(); i++)
			{
				Actor->GetAttachedShapes()[i]->SetPhysicalMaterial(*TDMaterial);//for now copy 
			}
		}

		Actor->GetFlags().SetFlagValue(TDActorFlags::EKinematic, BodyData.IsKinematic);

		Actor->GetFlags().SetFlagValue(TDActorFlags::ELockPosX, BodyData.LockXPosition);
		Actor->GetFlags().SetFlagValue(TDActorFlags::ELockPosY, BodyData.LockYPosition);
		Actor->GetFlags().SetFlagValue(TDActorFlags::ELockPosZ, BodyData.LockZPosition);
		
		Actor->GetFlags().SetFlagValue(TDActorFlags::ELockRotY, BodyData.LockYRot);
		Actor->GetFlags().SetFlagValue(TDActorFlags::ELockRotX, BodyData.LockXRot);
		Actor->GetFlags().SetFlagValue(TDActorFlags::ELockRotZ, BodyData.LockZRot);

		if (!FullRot.GetBoolValue())
		{
			//Todo:
			//This is on a Launch option as full rotations aren't stable enough
			Actor->GetFlags().SetFlagValue(TDActorFlags::ELockRotX, true);
			Actor->GetFlags().SetFlagValue(TDActorFlags::ELockRotZ, true);
		}		
	}
}

#endif