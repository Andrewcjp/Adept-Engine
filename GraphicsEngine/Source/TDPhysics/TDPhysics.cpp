
#include "TDPhysics.h"
#include "TDSolver.h"
#include "TDScene.h"
#include "TDVersion.h"
#include "Utils/MemoryUtils.h"
#include "TDSimConfig.h"
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

	Threading::TaskGraph * TDPhysics::GetTaskGraph()
	{
		return Instance->TDTaskGraph;
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
	}

	void TDPhysics::StartStep(float deltaTime)
	{
		for (int i = 0; i < Scenes.size(); i++)
		{
			Solver->ResolveCollisions(Scenes[i]);
			Solver->IntergrateScene(Scenes[i], deltaTime);
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

	TDSimConfig* TDPhysics::GetCurrentSimConfig()
	{
		if (Instance != nullptr)
		{
			return Instance->CurrentSimConfig;
		}
		return nullptr;
	}
}