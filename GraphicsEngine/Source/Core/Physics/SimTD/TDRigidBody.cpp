#if TDSIM_ENABLED
#include "TDRigidBody.h"
#include "TDRigidDynamic.h"
#include "TDPhysicsEngine.h"
#include "Shapes/TDSphere.h"

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
	return glm::vec3();
}

void TDRigidBody::AttachCollider(Collider * col)
{

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
	Actor->GetTransfrom()->SetPos(m_transform.GetPos());
	Actor->AttachShape(new TD::TDSphere());
	TDPhysicsEngine::GetScene()->AddToScene(Actor);
	CommonActorPTr = Actor;
}

void TDRigidBody::SetPositionAndRotation(glm::vec3 pos, glm::quat rot)
{
}

void TDRigidBody::SetGravity(bool state)
{
	if (Actor) 
	{
		Actor->SetGravity(state);
	}
}

#endif