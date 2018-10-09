#include "TDRigidDynamic.h"


namespace TD
{
	TDRigidDynamic::TDRigidDynamic()
	{
		ActorType = TDActorType::RigidDynamic;
		BodyMass = 10.0f;
	}

	TDRigidDynamic::~TDRigidDynamic()
	{}
	bool TDRigidDynamic::IsEffectedByGravity()
	{
		return UseGravity;
	}
	glm::vec3 TDRigidDynamic::GetVelocityDelta()
	{
		return DeltaVel;
	}
	void TDRigidDynamic::AddForce(glm::vec3 Force, bool AsForce)
	{
		if (AsForce)
		{
			DeltaVel = Force / BodyMass;
		}
		else
		{
			DeltaVel = Force;
		}

	}
	void TDRigidDynamic::ResetForceThisFrame()
	{
		DeltaVel = glm::vec3();
	}
	void TDRigidDynamic::SetGravity(bool enabled)
	{
		UseGravity = enabled;
	}
}