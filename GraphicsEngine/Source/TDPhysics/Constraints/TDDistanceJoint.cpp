#include "Source/TDPhysics/TDPCH.h"
#include "TDDistanceJoint.h"
namespace TD
{

	TDDistanceJoint::TDDistanceJoint(TDActor * BodyA, TDActor * BodyB, const ConstraintDesc & desc) : TDConstraint(BodyA, BodyB, desc)
	{}

	TDDistanceJoint::~TDDistanceJoint()
	{}

	void TDDistanceJoint::Resolve()
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

};