#include "TDRigidDynamic.h"
#include "TDPhysics.h"
#include "TDSimConfig.h"

namespace TD
{
	TDRigidDynamic::TDRigidDynamic()
	{
		ActorType = TDActorType::RigidDynamic;
		BodyMass = 10.0f;
		CachedsqSleepZeroThreshold = TDPhysics::GetCurrentSimConfig()->BodySleepZeroThreshold;
		CachedsqSleepZeroThreshold = CachedsqSleepZeroThreshold * CachedsqSleepZeroThreshold;//All compares are done Squared
	}

	TDRigidDynamic::~TDRigidDynamic()
	{}

	bool TDRigidDynamic::IsEffectedByGravity()
	{
		return UseGravity;
	}

	glm::vec3 TDRigidDynamic::GetLinearVelocityDelta()
	{
		return DeltaLinearVel;
	}

	glm::vec3 TDRigidDynamic::GetAngularVelocityDelta()
	{
		return DeltaAngularVel;
	}

	void TDRigidDynamic::AddForce(glm::vec3 Force, bool AsForce)
	{
		if (AsForce)
		{
			DeltaLinearVel = Force / BodyMass;
		}
		else
		{
			DeltaLinearVel = Force;
		}
	}
	bool TDRigidDynamic::CheckSleep(glm::vec3 & value)
	{
		if (glm::length2(value) <= CachedsqSleepZeroThreshold)
		{
			value = glm::vec3();
			return true;
		}
		return false;
	}
	void TDRigidDynamic::UpdateSleepTimer(float DT)
	{
		if (CheckSleep(DeltaLinearVel) && CheckSleep(DeltaAngularVel) && CheckSleep(LinearVelocity) && CheckSleep(AngularVel))
		{
			SleepTimer += DT;
		}
		else
		{
			SleepTimer = 0.0f;
		}
	}

	bool TDRigidDynamic::IsBodyAsleep() const
	{
		return SleepTimer > SleepMaxTimer;
	}

	void TDRigidDynamic::ResetForceThisFrame()
	{
		DeltaLinearVel = glm::vec3();
		DeltaAngularVel = glm::vec3();
	}

	void TDRigidDynamic::SetGravity(bool enabled)
	{
		UseGravity = enabled;
	}

	float TDRigidDynamic::GetBodyMass()
	{
		return BodyMass;
	}

	float TDRigidDynamic::GetInvBodyMass()
	{
		if (BodyMass <= FLT_EPSILON)
		{
			return 0.0f;
		}
		return 1.0f / BodyMass;
	}

	glm::vec3 TDRigidDynamic::GetLinearVelocity()
	{
		return LinearVelocity;
	}

	void TDRigidDynamic::SetLinearVelocity(glm::vec3 newvel)
	{
		LinearVelocity = newvel;
	}

	glm::vec3 TDRigidDynamic::GetAngularVelocity()
	{
		return AngularVel;
	}

	float TDRigidDynamic::GetInertiaTensor()
	{
		return InertaTensor;
	}

	void TDRigidDynamic::SetInertiaTensor(float tensor)
	{
		InertaTensor = tensor;
	}

	void TDRigidDynamic::SetAngularVelocity(glm::vec3 velocity)
	{
		AngularVel = velocity;
	}

	void TDRigidDynamic::AddTorque(glm::vec3 Torque)
	{
		DeltaAngularVel += Torque;
	}

	void TDRigidDynamic::AddForceAtPosition(glm::vec3 force)
	{
		//todo:
	}

}