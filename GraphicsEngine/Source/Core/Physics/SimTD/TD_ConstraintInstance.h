#pragma once
#include "TDPhysicsAPI.h"
#include "Physics/GenericConstraint.h"
class TD_ConstraintInstance: public GenericConstraint
{
public:
	TD_ConstraintInstance(TD::TDConstraint* con, ConstaintSetup setup);
	~TD_ConstraintInstance();

	virtual void UpdateConstraintData() override;
private:
	TD::TDConstraint* constraint = nullptr;

};

