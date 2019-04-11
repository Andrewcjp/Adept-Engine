
#include "TDConstraint.h"
namespace TD
{

	TDConstraint::TDConstraint(TDActor * bodyA, TDActor * bodyB, const ConstraintDesc & desc)
	{
		BodyA = bodyA;
		BodyB = bodyB;
		Desc = desc;
	}

	TDConstraint::~TDConstraint()
	{}
};