#include "GenericRigidBody.h"
#include "Core/Components/Component.h"
#include "Physics/PhysicsEngine.h"
GenericRigidBody::GenericRigidBody(EBodyType::Type type)
{
	BodyType = type;
}

GenericRigidBody::~GenericRigidBody()
{}

glm::vec3 GenericRigidBody::GetPosition() const
{
	return glm::vec3();
}

glm::quat GenericRigidBody::GetRotation() const
{
	return glm::quat();
}

void GenericRigidBody::AddTorque(glm::vec3 torque)
{

}

void GenericRigidBody::AddForce(glm::vec3 force, EForceMode::Type Mode)
{

}


glm::vec3 GenericRigidBody::GetLinearVelocity() const
{
	return glm::vec3();
}

void GenericRigidBody::SetLinearVelocity(glm::vec3 velocity)
{}

glm::vec3 GenericRigidBody::GetAngularVelocity() const
{
	return glm::vec3();
}

void GenericRigidBody::SetAngularVelocity(glm::vec3 velocity)
{}

glm::vec3 GenericRigidBody::GetInertiaTensor() const
{
	return glm::vec3();
}

void GenericRigidBody::SetInertiaTensor(float newtensor)
{}

void GenericRigidBody::SetGravity(bool active)
{
	BodyData.Gravity = active;
	UpdateBodyState();
}

void GenericRigidBody::AttachCollider(Collider * col)
{}

void GenericRigidBody::SetBodyData(BodyInstanceData data)
{
	BodyData = data;
	UpdateBodyState();
}

BodyInstanceData& GenericRigidBody::GetBodyData()
{
	return BodyData;
}

GenericCollider::GenericCollider()
{}

GenericCollider::~GenericCollider()
{}
GameObject * GenericCollider::GetGameObject()
{
	if (Owner != nullptr && Owner->GetOwnerComponent() != nullptr && Owner->GetOwnerComponent()->GetOwner() != nullptr)
	{
		return Owner->GetOwnerComponent()->GetOwner();
	}
	return nullptr;
}

void GenericCollider::SetOwner(RigidBody * newowner)
{
	Owner = newowner;
}

RigidBody * GenericCollider::GetOwner()
{
	return Owner;
}

void GenericCollider::SetEnabled(bool state)
{
	/*if (Shape == nullptr)
	{
		return;
	}*/
#if PHYSX_ENABLED
	if (state)
	{
		Shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		if (IsTrigger)
		{
			Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
			Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
		}
		else
		{
			Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
		}
	}
	else
	{
		Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
		Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
		Shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, false);
	}
#elif TDSIM_ENABLED
	if (state)
	{
		//Shape->GetFlags.SetFlags((physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		if (IsTrigger)
		{
			Shape->GetFlags().SetFlagValue(TD::TDShapeFlags::ESimulation, false);
			Shape->GetFlags().SetFlagValue(TD::TDShapeFlags::ETrigger, true);
		}
		else
		{
			Shape->GetFlags().SetFlagValue(TD::TDShapeFlags::ESimulation, true);
		}
	}
	else
	{
		Shape->GetFlags().SetFlagValue(TD::TDShapeFlags::ESimulation, false);
		Shape->GetFlags().SetFlagValue(TD::TDShapeFlags::ETrigger, false);
	}

#endif
}
