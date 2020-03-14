#include "GenericPhysicsEngine.h"
#include "GenericRigidBody.h"
#include "Core/GameObject.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Utils/TypeUtils.h"
static ConsoleVariable PHYS_DebugMode("physdebug", 0, ECVarType::ConsoleAndLaunch);

void GenericPhysicsEngine::InitPhysics()
{
	PHYS_DebugMode.SetValue(EPhysicsDebugMode::None);
}

void GenericPhysicsEngine::StepPhysics(float Deltatime)
{
	SetPhysicsDebugMode(PHYS_DebugMode.GetAsEnum<EPhysicsDebugMode::Type>());
}

void GenericPhysicsEngine::CleanupPhysics()
{}

bool GenericPhysicsEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* hit)
{
	return false;// RayCastScene(startpos, direction, distance, hit, std::vector<RigidBody*>());
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


EPhysicsDebugMode::Type GenericPhysicsEngine::GetCurrentMode()
{
	return EPhysicsDebugMode::None;
}