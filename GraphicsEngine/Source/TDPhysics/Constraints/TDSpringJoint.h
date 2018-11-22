#pragma once
#include "TDConstraint.h"
namespace TD
{
	class TDSpringJoint : public TDConstraint
	{
	public:
		TDSpringJoint(TDActor * BodyA, TDActor * BodyB, const ConstraintDesc & desc);
		~TDSpringJoint();

		virtual void Resolve() override;

	};

};
