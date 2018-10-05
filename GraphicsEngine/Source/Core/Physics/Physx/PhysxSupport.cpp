#include "Stdafx.h"
#include "PhysxSupport.h"
#include "PhysxRigidbody.h"
void PhysxCallBackHandler::onContact(const PxContactPairHeader & PairHeader, const PxContactPair * Pairs, PxU32 NumPairs)
{
	for (unsigned int PairIdx = 0; PairIdx < NumPairs; PairIdx++)
	{
		PhysxRigidbody* RigidBodyA = nullptr;
		PhysxRigidbody* RigidBodyB = nullptr;
		if (Pairs[PairIdx].shapes[0]->userData != nullptr)
		{
			RigidBodyA = (PhysxRigidbody*)Pairs[PairIdx].shapes[0]->userData;
		}
		if (Pairs[PairIdx].shapes[1]->userData != nullptr)
		{
			RigidBodyB = (PhysxRigidbody*)Pairs[PairIdx].shapes[1]->userData;
		}
		if (RigidBodyB == nullptr || RigidBodyA == nullptr)
		{
			Log::LogMessage("untracked Collison ",Log::Severity::Warning);
			return;
		}
		std::string data = "Obj: " + glm::to_string(RigidBodyA->GetPosition()) + " collided with " + glm::to_string(RigidBodyB->GetPosition());
		Log::LogMessage(data);
	}
}

void PhysxCallBackHandler::onConstraintBreak(PxConstraintInfo * constraints, PxU32 count)
{
	DebugEnsure(false);
}

void PhysxCallBackHandler::onWake(PxActor ** actors, PxU32 count)
{
	DebugEnsure(false);
}

void PhysxCallBackHandler::onSleep(PxActor ** actors, PxU32 count)
{
	DebugEnsure(false);

}

void PhysxCallBackHandler::onTrigger(PxTriggerPair * pairs, PxU32 count)
{
	DebugEnsure(false);
}

void PhysxCallBackHandler::onAdvance(const PxRigidBody * const * bodyBuffer, const PxTransform * poseBuffer, const PxU32 count)
{
	DebugEnsure(false);
}
