#include "TDPCH.h"
#include "TDSolver.h"
#include "TDScene.h"
namespace TD
{
	TDSolver::TDSolver()
	{}
	
	TDSolver::~TDSolver()
	{}

	void TDSolver::IntergrateScene(TDScene* scene, float dt)
	{
		for (int i = 0; i < scene->GetActors().size(); i++)
		{
			IntergrateActor(scene->GetActors()[i]);
		}
	}
	void TDSolver::IntergrateActor(TDActor* actor)
	{

	}
	void TDSolver::ResolveCollisions(TDScene* scene)
	{

	}
}