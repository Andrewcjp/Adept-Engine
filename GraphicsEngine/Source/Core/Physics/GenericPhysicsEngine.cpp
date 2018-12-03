#include "GenericPhysicsEngine.h"
#include "GenericRigidBody.h"
#include "Core/GameObject.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Utils/TypeUtils.h"
static ConsoleVariable DebugMode("physdebug", 0, ECVarType::ConsoleAndLaunch);

void GenericPhysicsEngine::initPhysics()
{}

void GenericPhysicsEngine::stepPhysics(float Deltatime)
{
	SetPhysicsDebugMode(DebugMode.GetAsEnum<EPhysicsDebugMode::Type>());
}

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

void GenericPhysicsEngine::SetPhysicsDebugMode(EPhysicsDebugMode::Type mode)
{
	PhysicsDebugMode = mode;
}



