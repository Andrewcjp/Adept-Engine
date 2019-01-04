#pragma once
#include "Physics/Physics_fwd.h"
#if TDSIM_ENABLED
#include "TDTypes.h"
#endif
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
#if TDSIM_ENABLED //Todo: Abstract! For now TD is close Enough
	TD::ConstraintDesc Desc;
#endif
};

/*! This class is used as the compile time base class for the rigid body class as only one physics engine can be used at once*/
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

