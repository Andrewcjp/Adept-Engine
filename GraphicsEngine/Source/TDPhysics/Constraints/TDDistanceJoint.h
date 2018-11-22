#pragma once
#include "TDConstraint.h"
namespace TD
{
	class TDDistanceJoint : public TDConstraint
	{
	public:
		TDDistanceJoint(TDActor * BodyA, TDActor * BodyB, const ConstraintDesc & desc);
		~TDDistanceJoint();
		virtual void Resolve() override;

	};

};