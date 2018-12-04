
#include "TDPhysics.h"
#include "TDSolver.h"
#include "TDScene.h"
#include "TDVersion.h"
#include "Utils/MemoryUtils.h"
#include "TDSimConfig.h"
#include "Utils/Threading.h"
#include "Constraints/TDSpringJoint.h"
#include "Constraints/TDDistanceJoint.h"
#include "TDSimulationCallbacks.h"
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
		Callbacks->OnContact(*SimulationCallbackPairs.data(),(int) SimulationCallbackPairs.size());
	}

	TDPhysics::TDPhysics()
	{}

	TDPhysics::~TDPhysics()
	{}

	TDPhysics * TDPhysics::CreatePhysics(unsigned int BuildID, TDSimConfig* SimConfig /*= nullptr*/)
	{
		if (BuildID != TD_VERSION_NUMBER)
		{
			//todo: message!
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
		if (Callbacks == nullptr)
		{
			Callbacks = new TDSimulationCallbacks();
		}
	}

	void TDPhysics::StartStep(TDScene* scene, float TimeStep)
	{
		//for (int i = 0; i < Scenes.size(); i++)
		{
			Solver->ResolveCollisions(scene);
			Solver->IntergrateScene(scene, TimeStep);
#if !BUILD_FULLRELEASE
			scene->DebugRender();
#endif
		}
	}

	void TDPhysics::ShutDown()
	{
		TDTaskGraph->Shutdown();
		SafeDelete(TDTaskGraph);
		MemoryUtils::DeleteVector(Scenes);
		SafeDelete(Solver);
		SafeDelete(Instance);
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
		TDConstraint* constaint;
		if (desc.Type == TDConstraintType::Spring)
		{
			constaint = new TDSpringJoint(BodyA, BodyB, desc);
		}
		else
		{
			constaint = new TDDistanceJoint(BodyA, BodyB, desc);
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