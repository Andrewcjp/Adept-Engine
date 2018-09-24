#pragma once
#if TDSIM_ENABLED
#include <vector>
#include "glm\glm.hpp"
#include <iostream>
#include <vector>
#include "Physics/Physics_fwd.h"
#include "Physics/PhysicsTypes.h"
class GameObject;
class TDPhysicsEngine
{
public:
	void initPhysics();
	void stepPhysics(float Deltatime);
	void cleanupPhysics();

	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit, bool CastEdtiorScene);
	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * hit);

	CORE_API RigidBody * CreatePrimitiveRigidBody(glm::vec3 position, glm::vec3 velocity, float scale);
	
	//old:
	void AddBoxCollisionToEditor(GameObject * obj);
	CORE_API std::vector<RigidBody*> createStack(const glm::vec3 & t, int size, float halfExtent);
	CORE_API RigidBody * FirePrimitiveAtScene(glm::vec3 position, glm::vec3 velocity, float scale/*, PxGeometryType::Enum type*/);

private:


};
#endif
