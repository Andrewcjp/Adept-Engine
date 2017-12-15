#include "PhysicsEngine.h"
#include "RigidBody.h"



std::vector<RigidBody*> PhysicsEngine::createStack(const glm::vec3 &, int, float)
{
	return std::vector<RigidBody*>();
}
RigidBody * PhysicsEngine::FirePrimitiveAtScene(glm::vec3, glm::vec3, float)
{
	return nullptr;
}
bool PhysicsEngine::RayCastScene(glm::vec3, glm::vec3, float, RayHit *hit)
{
	hit = nullptr;
	return false;
}
PhysicsEngine::PhysicsEngine()
{
}


PhysicsEngine::~PhysicsEngine()
{
}

void PhysicsEngine::initPhysics(bool)
{
}

void PhysicsEngine::stepPhysics(bool, float)
{
}

void PhysicsEngine::cleanupPhysics(bool)
{
}

RigidBody * PhysicsEngine::CreatePrimitiveRigidBody(glm::vec3, glm::vec3, float)
{
	return nullptr;
}

