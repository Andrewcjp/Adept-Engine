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



