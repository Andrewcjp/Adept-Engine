#pragma once
#include <include/physx/PxPhysicsAPI.h>
using namespace physx;
class PhysxCallBackHandler :public physx::PxSimulationEventCallback
{
	// Inherited via PxSimulationEventCallback
	void onContact(const PxContactPairHeader& PairHeader, const PxContactPair* Pairs, PxU32 NumPairs);
	
	virtual void onConstraintBreak(PxConstraintInfo * constraints, PxU32 count) override;

	virtual void onWake(PxActor ** actors, PxU32 count) override;

	virtual void onSleep(PxActor ** actors, PxU32 count) override;

	virtual void onTrigger(PxTriggerPair * pairs, PxU32 count) override;

	virtual void onAdvance(const PxRigidBody * const * bodyBuffer, const PxTransform * poseBuffer, const PxU32 count) override;

};
