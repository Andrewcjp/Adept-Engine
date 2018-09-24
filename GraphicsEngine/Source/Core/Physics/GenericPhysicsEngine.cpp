#include "GenericPhysicsEngine.h"
#include "GenericRigidBody.h"
#include <thread>
#include "Core/GameObject.h"
#include <algorithm>

void GenericPhysicsEngine::initPhysics()
{}

void GenericPhysicsEngine::stepPhysics(float Deltatime)
{}

void GenericPhysicsEngine::cleanupPhysics()
{}


RigidBody* GenericPhysicsEngine::CreatePrimitiveRigidBody(glm::vec3 position, glm::vec3 velocity, float scale)
{
	return nullptr;
}

bool GenericPhysicsEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* hit)
{
	return RayCastScene(startpos, direction, distance, hit, false);
}

bool GenericPhysicsEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* outhit, bool CastEdtiorScene)
{
	return false;
}

void GenericPhysicsEngine::AddBoxCollisionToEditor(GameObject* obj)
{

}

std::vector<RigidBody*> GenericPhysicsEngine::createStack(const glm::vec3 & t, int size, float halfExtent)
{
	return std::vector<RigidBody*>();
}


RigidBody * GenericPhysicsEngine::FirePrimitiveAtScene(glm::vec3 position, glm::vec3 velocity, float scale)
{
	return nullptr;
}