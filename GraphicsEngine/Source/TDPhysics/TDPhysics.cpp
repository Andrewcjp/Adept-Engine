#include "Stdafx.h"
#include "TDPhysics.h"
#include "TDSolver.h"
#include "TDScene.h"
#include "TDVersion.h"
#include "Utils/MemoryUtils.h"
#include "TDSimConfig.h"
namespace TD
{
	TDPhysics* TDPhysics::Instance = nullptr;
	TDPhysics::TDPhysics()
	{

	}

	TDPhysics::~TDPhysics()
	{

	}

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