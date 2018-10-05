#include "Stdafx.h"
#if PHYSX_ENABLED
#include "PhysxRigidbody.h"
#include "PhysxEngine.h"
#include "PhysxCollider.h"
#include "Physics/PhysicsTypes.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Engine.h"
using namespace physx;
PhysxRigidbody::~PhysxRigidbody()
{
	//PMaterial->release(); //Not released as we don't own it the shape does
	PhysxEngine::GetPlayScene()->removeActor(*CommonActorPtr);
	CommonActorPtr->release();
}

PhysxRigidbody::PhysxRigidbody(EBodyType::Type type, Transform initalpos) :GenericRigidBody(type)
{
	transform = initalpos;
}

glm::vec3 PhysxRigidbody::GetPosition()
{
	return PXvec3ToGLM(CommonActorPtr->getGlobalPose().p);
}

glm::quat PhysxRigidbody::GetRotation()
{
	return PXquatToGLM(CommonActorPtr->getGlobalPose().q);
}

void PhysxRigidbody::SetPositionAndRotation(glm::vec3 pos, glm::quat rot)
{
	if (CommonActorPtr != nullptr)
	{
		CommonActorPtr->setGlobalPose(PxTransform(GLMtoPXvec3(pos), GLMtoPXQuat(rot)));
	}
}

void PhysxRigidbody::AddTorque(glm::vec3 torque)
{
	if (Dynamicactor != nullptr)
	{
		Dynamicactor->addTorque(GLMtoPXvec3(torque));
	}
}

void PhysxRigidbody::AddForce(glm::vec3 force, EForceMode::Type Mode)
{
	if (Dynamicactor != nullptr)
	{
		Dynamicactor->addForce(GLMtoPXvec3(force));
	}
}

glm::vec3 PhysxRigidbody::GetLinearVelocity()
{
	if (Dynamicactor != nullptr)
	{
		return PXvec3ToGLM(Dynamicactor->getLinearVelocity());
	}
	return glm::vec3();
}

void PhysxRigidbody::SetLinearVelocity(glm::vec3 velocity)
{
	if (Dynamicactor != nullptr)
	{
		Dynamicactor->setLinearVelocity(GLMtoPXvec3(velocity));
	}
}

void PhysxRigidbody::AttachCollider(Collider * col)
{
	PMaterial = PhysxEngine::GetDefaultMaterial();
	PxShape* newShape = nullptr;
	for (int i = 0; i < col->Shapes.size(); i++)
	{
		ShapeElem* Shape = col->Shapes[i];
		switch (Shape->GetType())
		{
		case EShapeType::eBOX:
		{
			BoxElem* BoxShape = (BoxElem*)Shape;
			newShape = PhysxEngine::GetGPhysics()->createShape(PxBoxGeometry(GLMtoPXvec3(BoxShape->Extents)), *PMaterial);
			break;
		}
		case EShapeType::eSPHERE:
		{
			SphereElem* SphereShape = (SphereElem*)Shape;
			newShape = PhysxEngine::GetGPhysics()->createShape(PxSphereGeometry(SphereShape->raduis), *PMaterial);
			break;
		}
		}
		ensure(newShape);
		Shapes.push_back(newShape);
	}
}

void PhysxRigidbody::SetPhysicalMaterial(PhysicalMaterial * newmat)
{
	if (newmat != nullptr && newmat != PhysicsMat)
	{
		PhysicsMat = newmat;
		PMaterial = Engine::GetPhysEngineInstance()->CreatePhysxMat(PhysicsMat);
		for (int i = 0; i < Shapes.size(); i++)
		{
			Shapes[i]->setMaterials(&PMaterial, 1);
		}
	}
}

void PhysxRigidbody::UpdateFlagStates()
{
	if (Dynamicactor)
	{
		Dynamicactor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, LockData.LockXRot);
		Dynamicactor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, LockData.LockYRot);
		Dynamicactor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, LockData.LockZRot);
		Dynamicactor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, LockData.LockXPosition);
		Dynamicactor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, LockData.LockYPosition);
		Dynamicactor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, LockData.LockZPosition);
	}
}

void PhysxRigidbody::InitBody()
{
	if (PhysicsMat != nullptr)
	{
		PMaterial = Engine::GetPhysEngineInstance()->CreatePhysxMat(PhysicsMat);
	}
	else
	{
		PhysicsMat = PhysicalMaterial::GetDefault();
	}

	if (BodyType == EBodyType::RigidDynamic)
	{
		Dynamicactor = PhysxEngine::GetGPhysics()->createRigidDynamic(PxTransform(GLMtoPXvec3(transform.GetPos()), GLMtoPXQuat(transform.GetQuatRot())));
		for (int i = 0; i < Shapes.size(); i++)
		{
			Dynamicactor->attachShape(*Shapes[i]);
			Shapes[i]->userData = this;
		}
		Dynamicactor->setAngularDamping(LockData.AngularDamping);
		Dynamicactor->setLinearDamping(LockData.LinearDamping);
		if (LockData.UseAutoMass)
		{
			Dynamicactor->setMass(LockData.Mass);
		}
		else
		{
			PxRigidBodyExt::updateMassAndInertia(*Dynamicactor, PhysicsMat->density);
		}
		CommonActorPtr = Dynamicactor;
		UpdateFlagStates();
	}
	else if (BodyType == EBodyType::RigidStatic)
	{
		StaticActor = PhysxEngine::GetGPhysics()->createRigidStatic(PxTransform(GLMtoPXvec3(transform.GetPos()), GLMtoPXQuat(transform.GetQuatRot())));
		for (int i = 0; i < Shapes.size(); i++)
		{
			StaticActor->attachShape(*Shapes[i]);
			Shapes[i]->userData = this;
		}
		CommonActorPtr = StaticActor;
	}
	//CommonActorPtr->userData = this;
	PhysxEngine::GetPlayScene()->addActor(*CommonActorPtr);
}
#endif