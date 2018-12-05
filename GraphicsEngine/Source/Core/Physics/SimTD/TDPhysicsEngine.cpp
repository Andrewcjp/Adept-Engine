#include "TDPhysicsEngine.h"
#if TDSIM_ENABLED
#include "Core/GameObject.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Utils/DebugDrawers.h"
#include "Physics/GenericConstraint.h"
#include "Shapes/TDPlane.h"
#include "TDRigidBody.h"
#include "TDSimConfig.h"
#include "Editor/EditorWindow.h"
#include "TDSupport.h"
TDPhysicsEngine* TDPhysicsEngine::Instance = nullptr;
void TDPhysicsEngine::initPhysics()
{
	//TDSolver
	TDSimConfig* config = new TDSimConfig();
	config->PerfCounterCallBack = (TDSimConfig::FPrefCounterCallBack)&TimerCallbackHandler;
	config->DebugLineCallBack = (TDSimConfig::FDebugLineCallBack)&DebugLineCallbackHandler;
	config->CallBackHandler = new TDSupport();
	TDPhysics::CreatePhysics(TD_VERSION_NUMBER, config);
	TDPhysics::Get()->StartUp();
	Instance = this;
	PlayScene = TDPhysics::Get()->CreateScene();
	EditorScene = TDPhysics::Get()->CreateScene();
	TDRigidStatic* Actor = new TD::TDRigidStatic();
	Actor->GetTransfrom()->SetPos(glm::vec3(0, 0, 0));
	Actor->AttachShape(new TDPlane());
	//TDPhysicsEngine::GetScene()->AddToScene(Actor);
	DECLARE_TIMER_GROUP(GROUP_PhysicsEngine, "Physics Engine");
	PerfManager::Get()->AddTimer("ResolveCollisions", GROUP_PhysicsEngine);
	PerfManager::Get()->AddTimer("ResolveConstraints", GROUP_PhysicsEngine);
	PerfManager::Get()->AddTimer("IntergrateScene", GROUP_PhysicsEngine);
	PerfManager::Get()->AddTimer("IntersectionTests", GROUP_PhysicsEngine);

}
ConstraintInstance * TDPhysicsEngine::CreateConstraint(RigidBody * A, RigidBody * B, const ConstaintSetup& Setup)
{
	TD::ConstraintDesc desc;
	desc.Type = TDConstraintType::Spring;
	TDPhysics::Get()->CreateConstraint(A->GetActor(), B->GetActor(), desc);
	return nullptr;
}
void TDPhysicsEngine::DebugLineCallbackHandler(glm::vec3 start, glm::vec3 end, glm::vec3 Colour, float lifetime)
{
	DebugDrawers::DrawDebugLine(start, end, Colour, false, lifetime);
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
	case TD::TDPerfCounters::IntersectionTests:
		if (IsStart)
		{
			PerfManager::StartTimer("IntersectionTests");
		}
		else
		{
			PerfManager::EndTimer("IntersectionTests");
		}
		break;
	}
}

void TDPhysicsEngine::stepPhysics(float Deltatime)
{
	GenericPhysicsEngine::stepPhysics(Deltatime);
	TDPhysics::Get()->StartStep(PlayScene,Deltatime);
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
	if (data.BlockingHit)
	{
		outhit->Distance = data.Distance;
		outhit->Normal = data.Normal;
		outhit->position = data.Point;
		
		return true;
	}
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
#if WITH_EDITOR
	if (EditorWindow::GetInstance()->IsInPlayMode())
	{
		return Instance->PlayScene;
	}
	return Instance->EditorScene;
#else
	return Instance->PlayScene;
#endif	
}
TDPhysicsEngine * TDPhysicsEngine::Get()
{
	return Instance;
}

EPhysicsDebugMode::Type TDPhysicsEngine::GetCurrentMode()
{
	return Get()->PhysicsDebugMode;
}



#endif