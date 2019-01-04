#pragma once
#include "TDConstraint.h"

namespace TD
{
	class TDRigidDynamic;
	class TDSpringJoint : public TDConstraint
	{
	public:
		TDSpringJoint(TDActor * BodyA, TDActor * BodyB, const ConstraintDesc & desc);
		~TDSpringJoint();

		virtual void Resolve(float dt) override;

	private:
		TDRigidDynamic* ADynamic = nullptr;
		TDRigidDynamic* BDynamic = nullptr;
	};

};
