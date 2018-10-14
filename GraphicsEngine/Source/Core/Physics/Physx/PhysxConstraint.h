#pragma once
#include "Physics/Physics_fwd.h"
#include "Physics/GenericConstraint.h"
class PhysxConstraint : public GenericConstraint
{
public:
	PhysxConstraint(ConstaintSetup setup);
	~PhysxConstraint();
	virtual void UpdateConstraintData() override;
};

