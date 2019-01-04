#include "TDRigidDynamic.h"
#include "TDPhysics.h"
#include "TDSimConfig.h"
#include "TDSolver.h"
#include "TDActor.h"
#include "Shapes\TDSphere.h"
#include "Shapes\TDBox.h"

namespace TD
{
	TDRigidDynamic::TDRigidDynamic()
	{
		ActorType = TDActorType::RigidDynamic;
		CachedsqSleepZeroThreshold = TDPhysics::GetCurrentSimConfig()->BodySleepZeroThreshold;
		CachedsqSleepZeroThreshold = CachedsqSleepZeroThreshold * CachedsqSleepZeroThreshold;//All compares are done Squared
		ComputeInertiaTensor();
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

	void TDRigidDynamic::AddForce(glm::vec3 Force, TDForceMode::Type ForceMode)
	{
		glm::vec3 Addition = glm::vec3();
		if (ForceMode == TDForceMode::AsForce)
		{
			Addition = Force * GetInvBodyMass();
		}
		else if (ForceMode == TDForceMode::AsAcceleration)
		{
			Addition = Force;
		}
		DeltaLinearVel += Addition;
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

	void TDRigidDynamic::SetLinearDamping(float amt)
	{
		LinearDamping = amt;
	}

	float TDRigidDynamic::GetLinearDamping() const
	{
		return LinearDamping;
	}

#if VALIDATE_KE
	float TDRigidDynamic::Compute_KE()
	{
		return 0.5f* (GetBodyMass() * glm::length(GetLinearVelocity())*glm::length(GetLinearVelocity()));
	}

	void TDRigidDynamic::ComputeKE()
	{
		PreSimKE = Compute_KE();
	}

	void TDRigidDynamic::ValidateKE()
	{
		const float postsim = Compute_KE();
		//if (AttachedShapes.size() != 2)
		{
			float DelteE = (postsim - PreSimKE);

			if (postsim != PreSimKE)
			{
				/*printf("Delta E %f\n", DelteE);*/
			}
			//DebugEnsure(postsim == PreSimKE);
		}
	}
#endif
	void TDRigidDynamic::UpdateLockState()
	{
		if (ActorFlags.GetFlagValue(TDActorFlags::ELockPosX))
		{
			DeltaLinearVel.x = 0.0f;
		}
		if (ActorFlags.GetFlagValue(TDActorFlags::ELockPosY))
		{
			DeltaLinearVel.y = 0.0f;
		}
		if (ActorFlags.GetFlagValue(TDActorFlags::ELockPosZ))
		{
			DeltaLinearVel.z = 0.0f;
		}
#if ALLOW_ROT
		if (ActorFlags.GetFlagValue(TDActorFlags::ELockRotX))
		{
			DeltaAngularVel.x = 0.0f;
		}
		if (ActorFlags.GetFlagValue(TDActorFlags::ELockRotY))
		{
			DeltaAngularVel.y = 0.0f;
		}
		if (ActorFlags.GetFlagValue(TDActorFlags::ELockRotZ))
		{
			DeltaAngularVel.z = 0.0f;
		}
#endif
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
	{//todo: cache this
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
		//AngularVel = glm::vec3(0);
		return AngularVel;
	}

	void TDRigidDynamic::ComputeInertiaTensor()
	{
		if (BodyMass == 0.0f)
		{
			InertaTensor = glm::mat4(0);
		}
		float ix = 0.0f;
		float iy = 0.0f;
		float iz = 0.0f;
		float iw = 0.0f;

		if (true)//todo: check this
		{
			float r2 = 1.0f;
			if (AttachedShapes.size() > 0)
			{
				if (AttachedShapes[0]->GetShapeType() == TDShapeType::eSPHERE)
				{
					TDSphere* S = TDShape::CastShape<TDSphere>(AttachedShapes[0]);
					r2 = S->Radius* S->Radius;
					float fraction = (2.0f / 5.0f);

					ix = r2 * BodyMass * fraction;
					iy = r2 * BodyMass * fraction;
					iz = r2 * BodyMass * fraction;
					iw = 1.0f;
				}
				else if (AttachedShapes[0]->GetShapeType() == TDShapeType::eBOX)
				{
					TDBox* Box = TDShape::CastShape<TDBox>(AttachedShapes[0]);
					glm::vec3 size = Box->HalfExtends* 2.0f;
					float fraction = (1.0f / 12.0f);

					float x2 = size.x * size.x;
					float y2 = size.y * size.y;
					float z2 = size.z * size.z;

					ix = (y2 + z2) * GetBodyMass() * fraction;
					iy = (x2 + z2) * GetBodyMass() * fraction;
					iz = (x2 + y2) * GetBodyMass() * fraction;
					iw = 1.0f;
				}
			}
		}

		InertaTensor = glm::inverse(glm::mat4x4(
			ix, 0, 0, 0,
			0, iy, 0, 0,
			0, 0, iz, 0,
			0, 0, 0, iw));
	}

	glm::mat4x4 TDRigidDynamic::GetInertiaTensor()
	{
		ComputeInertiaTensor();
		return InertaTensor;
	}

	void TDRigidDynamic::SetInertiaTensor(glm::mat4x4 tensor)
	{
		InertaTensor = tensor;
	}

	void TDRigidDynamic::SetAngularVelocity(glm::vec3 velocity)
	{
		AngularVel = velocity;
	}

	void TDRigidDynamic::AddTorque(glm::vec3 Torque)
	{
		glm::vec3 angAccel = glm::vec4(Torque, 0.0f) * GetInertiaTensor();
		DeltaAngularVel += angAccel;
	}

	void TDRigidDynamic::AddForceAtPosition(glm::vec3 pos, glm::vec3 force)
	{
		glm::vec3 centerOfMass = GetTransfrom()->GetPos();
		glm::vec3 torque = glm::cross(pos - centerOfMass, force);
		AddTorque(torque);
		AddForce(force, TDForceMode::AsForce);
	}

	void TDRigidDynamic::SetBodyMass(float Mass)
	{
		BodyMass = Mass;
		ComputeInertiaTensor();
	}

}