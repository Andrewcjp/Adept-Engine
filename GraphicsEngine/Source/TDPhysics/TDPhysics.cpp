#include "Stdafx.h"
#include "TDPhysics.h"
#include "TDSolver.h"
#include "TDScene.h"
namespace TD
{
	TDPhysics* TDPhysics::Instance = nullptr;
	TDPhysics::TDPhysics()
	{

	}

	TDPhysics::~TDPhysics()
	{

	}

	TDPhysics * TDPhysics::CreatePhysics()
	{
		return new TDPhysics();
	}

	void TDPhysics::StartUp()
	{
		Solver = new TDSolver();
		CurrentScene = new TDScene();
	}

	void TDPhysics::StartStep(float deltaTime)
	{
		Solver->IntergrateScene(CurrentScene, deltaTime);
	}

	void TDPhysics::ShutDown()
	{
		SafeDelete(CurrentScene);
		SafeDelete(Solver);
	}
	TDScene * TDPhysics::GetScene()
	{
		return Instance->CurrentScene;
	}
	TDPhysics * TDPhysics::Get()
	{
		if (Instance == nullptr) 
		{
			Instance = new TDPhysics();
		}
		return Instance;
	}
}