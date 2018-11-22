#include "Source/TDPhysics/TDPCH.h"
#include "TDSpringJoint.h"
namespace TD
{

	TDSpringJoint::TDSpringJoint(TDActor * BodyA, TDActor * BodyB, const ConstraintDesc & desc) : TDConstraint(BodyA, BodyB, desc)
	{

	}

	TDSpringJoint::~TDSpringJoint()
	{}

	void TDSpringJoint::Resolve()
	{

	}

};