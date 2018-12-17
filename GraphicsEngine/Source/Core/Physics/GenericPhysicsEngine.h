#pragma once
#include "Physics/Physics_fwd.h"
#include "Physics/PhysicsTypes.h"
#include "GenericConstraint.h"
namespace EPhysicsDebugMode
{
	enum Type
	{
		None,
		ShowShapes,
		ShowContacts,
		ShowBroadPhaseShapes,//only valid for TDPhysics 
		All,
		Limit
	};
}
class GameObject;
/*! This class is used as the compile time base class for the Physics Engine class as only one physics engine can be used at once*/
class GenericPhysicsEngine
{
public:
	void InitPhysics();
	void StepPhysics(float Deltatime);
	void CleanupPhysics();
	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit);
	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * hit, std::vector<RigidBody*>& IgnoredActors);
	void SetPhysicsDebugMode(EPhysicsDebugMode::Type mode);
	ConstraintInstance * CreateConstraint(RigidBody * A, RigidBody * B, const ConstaintSetup& Setup);
protected:
	EPhysicsDebugMode::Type PhysicsDebugMode = EPhysicsDebugMode::None;

};

