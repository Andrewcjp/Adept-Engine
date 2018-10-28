#include "TDPhysicsEngine.h"
#if TDSIM_ENABLED
#include "TDRigidBody.h"
#include <thread>
#include "Core/GameObject.h"
#include <algorithm>
#include "Shapes/TDPlane.h"
#include "TDSimConfig.h"
#include "Core/Performance/PerfManager.h"
TDPhysicsEngine* TDPhysicsEngine::Instance = nullptr;
void TDPhysicsEngine::initPhysics()
{
	//TDSolver
	TDSimConfig* config = new TDSimConfig();
	config->PerfCounterCallBack = (TDSimConfig::PrefCounterCallBack)&TimerCallbackHandler;
	TDPhysics::CreatePhysics(TD_VERSION_NUMBER, config);
	TDPhysics::Get()->StartUp();
	Instance = this;
	PlayScene = TDPhysics::Get()->CreateScene();
	TDRigidStatic* Actor = new TD::TDRigidStatic();
	Actor->GetTransfrom()->SetPos(glm::vec3(0, 0, 0));
	Actor->AttachShape(new TDPlane());
	TDPhysicsEngine::GetScene()->AddToScene(Actor);
	DECLARE_TIMER_GROUP(GROUP_PhysicsEngine, "Physics Engine");
	PerfManager::Get()->AddTimer("ResolveCollisions", GROUP_PhysicsEngine);
	PerfManager::Get()->AddTimer("ResolveConstraints", GROUP_PhysicsEngine);
	PerfManager::Get()->AddTimer("IntergrateScene", GROUP_PhysicsEngine);
}

void TDPhysicsEngine::TimerCallbackHandler(bool IsStart, TDPerfCounters::Type type)
{
	switch (type)
	{
	case TD::TDPerfCounters::ResolveCollisions:
		if (IsStart)
		{
			PerfManager::StartTimer("ResolveCollisions");
		}
		else
		{
			PerfManager::EndTimer("ResolveCollisions");
		}
		break;
	case TD::TDPerfCounters::ResolveConstraints:
		if (IsStart)
		{
			PerfManager::StartTimer("ResolveConstraints");
		}
		else
		{
			PerfManager::EndTimer("ResolveConstraints");
		}
		break;
	case TD::TDPerfCounters::IntergrateScene:
		if (IsStart)
		{
			PerfManager::StartTimer("IntergrateScene");
		}
		else
		{
			PerfManager::EndTimer("IntergrateScene");
		}
		break;
	}
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
	RaycastData data;
	PlayScene->RayCastScene(startpos, direction, distance, &data);
	if (data.BlockingHit)
	{
		return true;
	}
	return false;// RayCastScene(startpos, direction, distance, hit, false);
}
bool TDPhysicsEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit, std::vector<RigidBody*>& IgnoredActors)
{
	RaycastData data;
	PlayScene->RayCastScene(startpos, direction, distance, &data);
	return false;
}
bool TDPhysicsEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit, bool CastEdtiorScene, std::vector<RigidBody*>& IgnoredActors)
{

	return false;
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
TDScene * TDPhysicsEngine::GetScene()
{
	return Instance->PlayScene;
}
TDPhysicsEngine * TDPhysicsEngine::Get()
{
	return Instance;
}



#endif