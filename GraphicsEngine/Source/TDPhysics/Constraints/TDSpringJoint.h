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
		float restingLength = 5.0f;
		float b = 0.1f;
		float k = 1.0f;
	private:
		TDRigidDynamic* ADynamic = nullptr;
		TDRigidDynamic* BDynamic = nullptr;
	};

};
