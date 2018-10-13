#include "TDPhysicsEngine.h"
#if TDSIM_ENABLED
#include "TDRigidBody.h"
#include <thread>
#include "Core/GameObject.h"
#include <algorithm>
#include "TDPhysicsAPI.h"
void TDPhysicsEngine::initPhysics()
{
	//TDSolver
	TDPhysics::CreatePhysics(TD_VERSION_NUMBER);
	TDPhysics::Get()->StartUp();
}

void TDPhysicsEngine::stepPhysics(float Deltatime)
{
	TDPhysics::Get()->StartStep(Deltatime);
}

void TDPhysicsEngine::cleanupPhysics()
{}

RigidBody* TDPhysicsEngine::CreatePrimitiveRigidBody(glm::vec3 position, glm::vec3 velocity, float scale)
{
	return nullptr;
}

bool TDPhysicsEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* hit)
{
	return RayCastScene(startpos, direction, distance, hit, false);
}

bool TDPhysicsEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* outhit, bool CastEdtiorScene)
{
	return false;
}

void TDPhysicsEngine::AddBoxCollisionToEditor(GameObject* obj)
{

}

std::vector<RigidBody*> TDPhysicsEngine::createStack(const glm::vec3 & t, int size, float halfExtent)
{
	return std::vector<RigidBody*>();
}


RigidBody * TDPhysicsEngine::FirePrimitiveAtScene(glm::vec3 position, glm::vec3 velocity, float scale)
{
	return nullptr;
}
#endif