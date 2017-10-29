#include "PhysicsEngine.h"
#include "RigidBody.h"



std::vector<RigidBody*> PhysicsEngine::createStack(const glm::vec3 & t, int size, float halfExtent)
{
	return std::vector<RigidBody*>();
}
RigidBody * PhysicsEngine::FirePrimitiveAtScene(glm::vec3 position, glm::vec3 velocity, float scale)
{
	return nullptr;
}
bool PhysicsEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * hit)
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

void PhysicsEngine::initPhysics(bool interactive)
{
}

void PhysicsEngine::stepPhysics(bool interactive, float Deltatime)
{
}

void PhysicsEngine::cleanupPhysics(bool interactive)
{
}

