#include "GenericPhysicsEngine.h"
#include "GenericRigidBody.h"
#include "Core/GameObject.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Utils/TypeUtils.h"
static ConsoleVariable DebugMode("physdebug", 0, ECVarType::ConsoleAndLaunch);

void GenericPhysicsEngine::InitPhysics()
{
	DebugMode.SetValue(EPhysicsDebugMode::ShowBroadPhaseShapes); 
}

void GenericPhysicsEngine::StepPhysics(float Deltatime)
{
	SetPhysicsDebugMode(DebugMode.GetAsEnum<EPhysicsDebugMode::Type>());
}

void GenericPhysicsEngine::CleanupPhysics()
{}

bool GenericPhysicsEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* hit)
{
	return RayCastScene(startpos, direction, distance, hit, std::vector<RigidBody*>());
}

bool GenericPhysicsEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * hit, std::vector<RigidBody*>& IgnoredActors)
{
	return false;
}

void GenericPhysicsEngine::SetPhysicsDebugMode(EPhysicsDebugMode::Type mode)
{
	PhysicsDebugMode = mode;
}

ConstraintInstance * GenericPhysicsEngine::CreateConstraint(RigidBody * A, RigidBody * B, const ConstaintSetup & Setup)
{
	return nullptr;
}


