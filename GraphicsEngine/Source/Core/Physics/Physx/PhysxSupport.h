#pragma once
#if PHYSX_ENABLED
#include "Physics/Physics_fwd.h"
#include <PxPhysicsAPI.h>
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
class FPxQueryFilterCallback : public PxQueryFilterCallback
{
public:

	std::vector<RigidBody*> IgnoredBodies;

	/** Result of PreFilter callback. */
	PxQueryHitType::Enum PrefilterReturnValue;

	/** Whether we are doing an overlap query. This is needed to ensure physx results are never blocking (even if they are in terms of unreal)*/
	bool bIsOverlapQuery;

	/** Whether to ignore touches (convert an eTOUCH result to eNONE). */
	bool bIgnoreTouches;

	/** Whether to ignore blocks (convert an eBLOCK result to eNONE). */
	bool bIgnoreBlocks;

	FPxQueryFilterCallback()
	{
		PrefilterReturnValue = PxQueryHitType::eNONE;
		bIsOverlapQuery = false;

	}

	/**
	 * Calculate Result Query HitType from Query Filter and Shape Filter
	 *
	 * @param PQueryFilter	: Querier FilterData
	 * @param PShapeFilter	: The Shape FilterData querier is testing against
	 *
	 * @return PxQueryHitType from both FilterData
	 */
	static PxQueryHitType::Enum CalcQueryHitType(const PxFilterData &PQueryFilter, const PxFilterData &PShapeFilter, bool bPreFilter = false);

	virtual PxQueryHitType::Enum preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags) override;


	virtual PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit) override
	{
		// Currently not used
		return PxQueryHitType::eBLOCK;
	}
};




#endif
