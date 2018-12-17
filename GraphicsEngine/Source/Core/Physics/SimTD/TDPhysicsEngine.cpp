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
void TDPhysicsEngine::InitPhysics()
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
	Actor->GetTransfrom()->SetPos(glm::vec3(0, 1, 0));
	Actor->AttachShape(new TDPlane());
	//PlayScene->AddToScene(Actor);
	
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

void TDPhysicsEngine::StepPhysics(float Deltatime)
{
	GenericPhysicsEngine::StepPhysics(Deltatime);
	TDPhysics::Get()->StartStep(PlayScene, Deltatime);
}

void TDPhysicsEngine::CleanupPhysics()
{}


bool TDPhysicsEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* hit)
{
	return RayCastScene(startpos, direction, distance, hit, std::vector<RigidBody*>());
}

bool TDPhysicsEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit, std::vector<RigidBody*>& IgnoredActors)
{
	RaycastData data;
	GetScene()->RayCastScene(startpos, direction, distance, &data);
	if (data.BlockingHit)
	{
		outhit->Distance = data.Points[0].Distance;
		outhit->Normal = data.Points[0].Normal;
		outhit->position = data.Points[0].Point;

		return true;
	}
	return false;
}

TDScene * TDPhysicsEngine::GetScene()
{
#if WITH_EDITOR
	if (EditorWindow::GetInstance() != nullptr && EditorWindow::GetInstance()->IsInPlayMode())
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