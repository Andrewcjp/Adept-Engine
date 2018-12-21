#pragma once
#include "TDTypes.h"
namespace TD
{
	//this contains all a constraints data and solving logic
	class TDActor;
	class TDConstraint
	{
	public:
		TDConstraint(TDActor * BodyA, TDActor * BodyB, const ConstraintDesc & desc);
		virtual ~TDConstraint();
		virtual void Resolve(float dt);
	protected:
		TDActor* BodyA = nullptr;
		TDActor* BodyB = nullptr;
		ConstraintDesc Desc;
	};
};

