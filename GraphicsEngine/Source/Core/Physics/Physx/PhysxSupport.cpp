
#include "PhysxSupport.h"
#include "PhysxRigidbody.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Components/Core_Components_inc.h"
#include "Core/GameObject.h"
#if PHYSX_ENABLED
void PhysxCallBackHandler::onContact(const PxContactPairHeader & PairHeader, const PxContactPair * Pairs, PxU32 NumPairs)
{
	for (unsigned int PairIdx = 0; PairIdx < NumPairs; PairIdx++)
	{
		Collider* ColliderA = nullptr;
		Collider* ColliderB = nullptr;
		if (Pairs[PairIdx].shapes[0]->userData != nullptr)
		{
			ColliderA = (Collider*)Pairs[PairIdx].shapes[0]->userData;
		}
		if (Pairs[PairIdx].shapes[1]->userData != nullptr)
		{
			ColliderB = (Collider*)Pairs[PairIdx].shapes[1]->userData;
		}
		if (ColliderB == nullptr || ColliderA == nullptr)
		{
			Log::LogMessage("untracked Collison ", Log::Severity::Warning);
			return;
		}
		/*std::string data = "Obj: " + glm::to_string(RigidBodyA->GetOwner()->GetPosition()) + " collided with " + glm::to_string(RigidBodyB->GetOwner()->GetPosition());
		Log::LogMessage(data);*/

		CollisonData Data;
		//todo: check order here!
		Data.Hitcollider = ColliderA;
		Data.OtherCollider = ColliderB;

		if (ColliderA->GetOwner() != nullptr && ColliderA->GetOwner()->GetOwnerComponent() != nullptr && ColliderA->GetOwner()->GetOwnerComponent()->GetOwner() != nullptr)
		{
			ColliderA->GetOwner()->GetOwnerComponent()->GetOwner()->BroadCast_OnCollide(Data);
		}
		Data.Hitcollider = ColliderB;
		Data.OtherCollider = ColliderA;
		if (ColliderB->GetOwner() != nullptr && ColliderB->GetOwner()->GetOwnerComponent() != nullptr && ColliderB->GetOwner()->GetOwnerComponent()->GetOwner() != nullptr)
		{
			ColliderB->GetOwner()->GetOwnerComponent()->GetOwner()->BroadCast_OnCollide(Data);
		}
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

void PhysxCallBackHandler::onTrigger(PxTriggerPair * Pairs, PxU32 count)
{
	for (unsigned int PairIdx = 0; PairIdx < count; PairIdx++)
	{
		Collider* ColliderA = nullptr;
		Collider* ColliderB = nullptr;
		if (Pairs[PairIdx].triggerShape->userData != nullptr)
		{
			ColliderA = (Collider*)Pairs[PairIdx].triggerShape->userData;
		}
		if (Pairs[PairIdx].otherShape->userData != nullptr)
		{
			ColliderB = (Collider*)Pairs[PairIdx].otherShape->userData;
		}
		if (ColliderB == nullptr || ColliderA == nullptr)
		{
			Log::LogMessage("untracked Trigger Collision ", Log::Severity::Warning);
			continue;
		}
		CollisonData Data;
		//todo: check order here!
		Data.Hitcollider = ColliderA;
		Data.OtherCollider = ColliderB;

		if (ColliderA->GetOwner() != nullptr && ColliderA->GetOwner()->GetOwnerComponent() != nullptr && ColliderA->GetOwner()->GetOwnerComponent()->GetOwner() != nullptr)
		{
			ColliderA->GetOwner()->GetOwnerComponent()->GetOwner()->BroadCast_OnTrigger(Data);
		}
		Data.Hitcollider = ColliderB;
		Data.OtherCollider = ColliderA;
		if (ColliderB->GetOwner() != nullptr && ColliderB->GetOwner()->GetOwnerComponent() != nullptr && ColliderB->GetOwner()->GetOwnerComponent()->GetOwner() != nullptr)
		{
			ColliderB->GetOwner()->GetOwnerComponent()->GetOwner()->BroadCast_OnTrigger(Data);
		}
	}
}

void PhysxCallBackHandler::onAdvance(const PxRigidBody * const * bodyBuffer, const PxTransform * poseBuffer, const PxU32 count)
{
	DebugEnsure(false);
}


PxQueryHitType::Enum FPxQueryFilterCallback::CalcQueryHitType(const PxFilterData & PQueryFilter, const PxFilterData & PShapeFilter, bool bPreFilter)
{
	return PxQueryHitType::Enum();
}

PxQueryHitType::Enum FPxQueryFilterCallback::preFilter(const PxFilterData & filterData, const PxShape * shape, const PxRigidActor * actor, PxHitFlags & queryFlags)
{
	// Check if the shape is the right complexity for the trace 
	RigidBody* rb = (RigidBody*)actor->userData;
	for (int i = 0; i < IgnoredBodies.size(); i++)
	{
		if (rb == IgnoredBodies[i])
		{
			return PxQueryHitType::eNONE;
		}
	}
	return PxQueryHitType::eBLOCK;
}
#endif