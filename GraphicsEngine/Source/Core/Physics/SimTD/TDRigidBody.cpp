#if TDSIM_ENABLED
#include "TDRigidBody.h"
#include "TDRigidDynamic.h"
#include "TDPhysicsEngine.h"
#include "Shapes/TDSphere.h"
#include "Shapes/TDBox.h"
#include "Shapes/TDMeshShape.h"
#include "Core/Utils/DebugDrawers.h"

TDRigidBody::TDRigidBody(EBodyType::Type type, Transform T) :GenericRigidBody(type)
{
	m_transform = T;
}

TDRigidBody::~TDRigidBody()
{
	TDPhysicsEngine::GetScene()->RemoveActor(Actor);
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
	Actor->AddForce(force, (Mode == EForceMode::AsForce));
}

glm::vec3 TDRigidBody::GetLinearVelocity()const
{
	return Actor->GetLinearVelocity();
}

void TDRigidBody::AttachCollider(Collider * col)
{
	//Actor->AttachShape(new TD::TDSphere());
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
			newShape = new TD::TDBox();
			break;
		}
		case EShapeType::eCAPSULE:
		case EShapeType::eSPHERE:
		{
			SphereElem* SphereShape = (SphereElem*)Shape;
			newShape = new TD::TDSphere();
			break;
		}
		case EShapeType::eTRIANGLEMESH:
		{
			TriMeshElm* SphereShape = (TriMeshElm*)Shape;
			newShape = new TD::TDMeshShape(GenerateTriangleMesh(SphereShape->MeshAssetName, glm::vec3(1)));
			break;
		}
		}
		shapes.push_back(newShape);
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

	desc.Indices.Count = indices.size() / 2;
	desc.Indices.Stride = sizeof(int);
	desc.Indices.DataPtr = indices.data();

	desc.HasPerVertexNormals = true;
	desc.Normals.Count = normals.size();
	desc.Normals.Stride = sizeof(glm::vec3);
	desc.Normals.DataPtr = normals.data();

	TD::TDMesh* newmesh = new TD::TDMesh(desc);
	newmesh->CookMesh();
	/*std::vector<TDTriangle*> tris = newmesh->GetTriangles();
	for (int i = 0; i < tris.size(); i++)
	{
		const int sides = 3;
		for (int x = 0; x < sides; x++)
		{
			const int next = (x + 1) % sides;
			DebugDrawers::DrawDebugLine(tris[i]->Points[x], tris[i]->Points[next], glm::vec3(1), false, 100.0f);
		}
	}*/


	return newmesh;
}
void TDRigidBody::SetBodyData(BodyInstanceData data)
{

}

BodyInstanceData TDRigidBody::GetBodyData()
{
	return BodyInstanceData();
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
	Actor = new TD::TDRigidDynamic();
	for (TD::TDShape* s : shapes)
	{
		Actor->AttachShape(s);
	}
	Actor->GetTransfrom()->SetPos(m_transform.GetPos());
	Actor->SetGravity(data.Gravity);
	TDPhysicsEngine::GetScene()->AddToScene(Actor);
	CommonActorPTr = Actor;
}

void TDRigidBody::SetPositionAndRotation(glm::vec3 pos, glm::quat rot)
{
	if (Actor)
	{
		Actor->GetTransfrom()->SetPos(pos);
		Actor->GetTransfrom()->SetQrot(rot);
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
		data.Gravity = state;
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

#endif