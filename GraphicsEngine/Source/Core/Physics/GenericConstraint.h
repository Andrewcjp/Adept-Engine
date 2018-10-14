#pragma once
#include "Physics/Physics_fwd.h"
namespace EConstaintMotion
{
	enum Type
	{
		Free,
		Limited,
		Locked,
	};
}
struct LinearConstaintSetup
{
	float limit = 0;
	EConstaintMotion::Type XMotion;
	EConstaintMotion::Type YMotion;
	EConstaintMotion::Type ZMotion;
};
struct ConstaintSetup
{
	float LinearBreakThreshold = 1.0f;
	float AngularBreakThreshold = 10.0f;
	LinearConstaintSetup LinearSetup;

	bool EnableProjection = false;
	float ProjectionLinearTolerance;
};
class GenericConstraint
{
public:
	GenericConstraint(ConstaintSetup setup);
	~GenericConstraint();
	virtual void UpdateConstraintData();
protected:
	ConstaintSetup CurrentSetup;
	RigidBody* BodyA = nullptr;
	RigidBody* BodyB = nullptr;
};

