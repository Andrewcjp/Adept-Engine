#pragma once

#include "EngineGlobals.h"
#include <vector>
#include "glm\glm.hpp"
class RigidBody;
typedef struct _RayHit
{
	glm::vec3 position;

}RayHit;
#if PHYSX_ENABLED
class PhysxEngine;
#endif
class PhysicsEngine
{
public:
	virtual std::vector<RigidBody*> createStack(const glm::vec3 & t, int size, float halfExtent);

	virtual RigidBody *FirePrimitiveAtScene(glm::vec3 position, glm::vec3 velocity, float scale/*, PxGeometryType::Enum type*/);
	virtual	bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* hit);
	PhysicsEngine();
	~PhysicsEngine();
	virtual void initPhysics(bool interactive);
	virtual void stepPhysics(bool interactive, float Deltatime);
	virtual void cleanupPhysics(bool interactive);
	virtual RigidBody* CreatePrimitiveRigidBody(glm::vec3 position, glm::vec3 velocity, float scale);
private:


};

