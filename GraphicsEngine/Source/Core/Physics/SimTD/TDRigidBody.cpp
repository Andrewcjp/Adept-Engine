#if TDSIM_ENABLED
#include "TDRigidBody.h"
#include "TDRigidDynamic.h"
#include "TDPhysicsEngine.h"
#include "Shapes/TDSphere.h"
#include "Shapes/TDBox.h"

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
		case EShapeType::eSPHERE:
		{
			SphereElem* SphereShape = (SphereElem*)Shape;
			newShape = new TD::TDSphere();
			break;
		}
		}
		shapes.push_back(newShape);
	}	
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

}

void TDRigidBody::InitBody()
{
	Actor = new TD::TDRigidDynamic();
	for (TD::TDShape* s : shapes)
	{
		Actor->AttachShape(s);
	}
	Actor->GetTransfrom()->SetPos(m_transform.GetPos());
	TDPhysicsEngine::GetScene()->AddToScene(Actor);
	CommonActorPTr = Actor;
}

void TDRigidBody::SetPositionAndRotation(glm::vec3 pos, glm::quat rot)
{}

void TDRigidBody::SetGravity(bool state)
{
	if (Actor)
	{
		Actor->SetGravity(state);
	}
}

#endif