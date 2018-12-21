#include "Source/TDPhysics/TDPCH.h"
#include "TDDistanceJoint.h"
#include "TDActor.h"
#include "TDTransform.h"
namespace TD
{

	TDDistanceJoint::TDDistanceJoint(TDActor * BodyA, TDActor * BodyB, const ConstraintDesc & desc) : TDConstraint(BodyA, BodyB, desc)
	{}

	TDDistanceJoint::~TDDistanceJoint()
	{}

	void TDDistanceJoint::Resolve(float dt)
	{
		BodyB->GetTransfrom()->SetPos(glm::vec3(0, 15, 0));
		BodyA->GetTransfrom()->SetPos(glm::vec3(0, 10, 0));
	}

};