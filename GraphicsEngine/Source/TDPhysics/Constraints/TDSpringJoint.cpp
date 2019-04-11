#include "TDSpringJoint.h"
#include "TDActor.h"
#include "TDTransform.h"
#include "TDRigidDynamic.h"
namespace TD
{

	TDSpringJoint::TDSpringJoint(TDActor * BodyA, TDActor * BodyB, const ConstraintDesc & desc) : TDConstraint(BodyA, BodyB, desc)
	{
		ADynamic = TDActor::ActorCast<TDRigidDynamic>(BodyA);
		BDynamic = TDActor::ActorCast<TDRigidDynamic>(BodyB);
	}

	TDSpringJoint::~TDSpringJoint()
	{}

	void TDSpringJoint::Resolve(float dt)
	{
		glm::vec3 relPos = BodyA->GetTransfrom()->GetPos() - BodyB->GetTransfrom()->GetPos();
		glm::vec3 relVel = glm::vec3();
		if (ADynamic != nullptr && BDynamic != nullptr)
		{
			relVel = ADynamic->GetLinearVelocity() - BDynamic->GetLinearVelocity();
		}
		else if (ADynamic != nullptr)
		{
			relVel = ADynamic->GetLinearVelocity();
		}
		else  if (BDynamic != nullptr)
		{
			relVel = BDynamic->GetLinearVelocity();
		}	

		float x = glm::length(relPos) - Desc.RestLength;
		float v = glm::length(relVel);
		//Standard Spring Equation!
		float F = (-Desc.SpringK * x) +(-Desc.Dampening * v);
		glm::vec3 impulse = glm::normalize(relPos) * F;
		if (ADynamic != nullptr)
		{
			ADynamic->SetLinearVelocity(ADynamic->GetLinearVelocity() + (impulse * ADynamic->GetInvBodyMass()));
		}
		if (BDynamic != nullptr)
		{
			BDynamic->SetLinearVelocity(BDynamic->GetLinearVelocity() + (impulse * -1.0f * BDynamic->GetInvBodyMass()));
		}
	}
};