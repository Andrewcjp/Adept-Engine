#pragma once
#include "Physics/Physics_fwd.h"
#include "Physics/GenericConstraint.h"
class PhysxConstraint : public GenericConstraint
{
public:
	PhysxConstraint(ConstaintSetup setup);
	virtual ~PhysxConstraint();
	virtual void UpdateConstraintData() override;
};

