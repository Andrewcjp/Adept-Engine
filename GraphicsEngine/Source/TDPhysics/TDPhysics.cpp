#include "TDPhysics.h"
#include "Constraints/TDDistanceJoint.h"
#include "Constraints/TDSpringJoint.h"
#include "TDScene.h"
#include "TDSimConfig.h"
#include "TDSimulationCallbacks.h"
#include "TDSolver.h"
#include "TDVersion.h"
#include "Utils/MemoryUtils.h"
#include "Utils/Threading.h"

namespace TD
{
	TDPhysics* TDPhysics::Instance = nullptr;
	void TDPhysics::StartTimer(TDPerfCounters::Type timer)
	{
		if (Get()->GetCurrentSimConfig()->PerfCounterCallBack != nullptr)
		{
			Get()->GetCurrentSimConfig()->PerfCounterCallBack(true, timer);
		}
	}

	void TDPhysics::EndTimer(TDPerfCounters::Type timer)
	{
		if (Get()->GetCurrentSimConfig()->PerfCounterCallBack != nullptr)
		{
			Get()->GetCurrentSimConfig()->PerfCounterCallBack(false, timer);
		}
	}

	void TDPhysics::DrawDebugLine(glm::vec3 LineStart, glm::vec3 LineEnd, glm::vec3 Colour, float lifetime)
	{
		//return;
		if (Get()->GetCurrentSimConfig()->DebugLineCallBack != nullptr)
		{
			Get()->GetCurrentSimConfig()->DebugLineCallBack(LineStart, LineEnd, Colour, lifetime);
		}
	}

	void TDPhysics::DrawDebugPoint(glm::vec3 pos, glm::vec3 colour, float Lifetime)
	{
		DrawDebugLine(pos, pos + glm::vec3(0, 1, 0), colour, Lifetime);
	}

	Threading::TaskGraph * TDPhysics::GetTaskGraph()
	{
		return Instance->TDTaskGraph;
	}

	void TDPhysics::SimulationContactCallback(std::vector<ContactPair*>& SimulationCallbackPairs)
	{		
		if (SimulationCallbackPairs.size() != 0)
		{
			Callbacks->OnContact((const ContactPair**)SimulationCallbackPairs.data(), (int)SimulationCallbackPairs.size());
		}
	}

	void TDPhysics::TriggerSimulationContactCallback(std::vector<ContactPair*>& SimulationCallbackPairs)
	{
		if (SimulationCallbackPairs.size() != 0)
		{
			Callbacks->OnTrigger((const ContactPair**)SimulationCallbackPairs.data(), (int)SimulationCallbackPairs.size());//todo: remove cast!
		}
	}

	void TDPhysics::SetDebugShowBroadPhaseShapes(bool state)
	{
		GetCurrentSimConfig()->ShowBroadPhaseShapes = state;
	}

	void TDPhysics::SetDebugShowContacts(bool state)
	{
		GetCurrentSimConfig()->ShowContacts = state;
	}

	TDPhysics::TDPhysics()
	{}

	TDPhysics::~TDPhysics()
	{}

	TDPhysics * TDPhysics::CreatePhysics(unsigned int BuildID, TDSimConfig* SimConfig /*= nullptr*/)
	{
		if (BuildID != TD_VERSION_NUMBER)
		{
			printf("BuildID Incorrect Init Failed");
			return nullptr;
		}
		if (Instance == nullptr)
		{
			Instance = new TDPhysics();
			Instance->CurrentSimConfig = SimConfig;
			if (Instance->CurrentSimConfig == nullptr)
			{
				Instance->CurrentSimConfig = new TDSimConfig();
			}
		}
		return Instance;
	}

	void TDPhysics::StartUp()
	{
		TDTaskGraph = new Threading::TaskGraph(CurrentSimConfig->TaskGraphThreadCount);
		Solver = new TDSolver();
		Callbacks = GetCurrentSimConfig()->CallBackHandler;
		if (Callbacks == nullptr)
		{
			Callbacks = new TDSimulationCallbacks();
		}
	}

	void TDPhysics::StartStep(TDScene* scene, float TimeStep)
	{
			Solver->FinishAccumlateForces(scene);
			Solver->ResolveCollisions(scene);
			Solver->IntergrateScene(scene, TimeStep);
#if BUILD_DEBUG_RENDER
			scene->DebugRender();
#endif
	}

	void TDPhysics::ShutDown()
	{
		TDTaskGraph->Shutdown();
		SafeDelete(TDTaskGraph);
		MemoryUtils::DeleteVector(Scenes);
		SafeDelete(Solver);
		SafeDelete(Callbacks);
		SafeDelete(Instance);//This is the same as Delete THIS		
	}

	TDPhysics * TDPhysics::Get()
	{
		return Instance;
	}

	TDScene * TDPhysics::CreateScene()
	{
		TDScene* newscene = new TDScene();
		Scenes.push_back(newscene);
		return newscene;
	}

	TDConstraint * TDPhysics::CreateConstraint(TDActor * BodyA, TDActor * BodyB, const ConstraintDesc & desc)
	{
		if (BodyA->GetScene() != BodyB->GetScene() || BodyA->GetScene() == nullptr || BodyB->GetScene() == nullptr)
		{
			return nullptr;
		}
		TDConstraint* constaint = nullptr;
		if (desc.Type == TDConstraintType::Spring)
		{
			constaint = new TDSpringJoint(BodyA, BodyB, desc);
		}
		else if(desc.Type == TDConstraintType::Distance)
		{
			constaint = new TDDistanceJoint(BodyA, BodyB, desc);
		}
		if (constaint == nullptr)
		{
			return nullptr;
		}
		BodyA->GetScene()->AddConstraint(constaint);
		return constaint;
	}

	TDSimConfig* TDPhysics::GetCurrentSimConfig()
	{
		if (Instance != nullptr)
		{
			return Instance->CurrentSimConfig;
		}
		return nullptr;
	}
}