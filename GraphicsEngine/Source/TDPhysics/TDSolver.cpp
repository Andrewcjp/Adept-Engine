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
			IntergrateActor(scene->GetActors()[i], dt, scene);
		}
	}

	void TDSolver::IntergrateActor(TDActor* actor, float dt, TDScene* Scene)
	{
		glm::vec3 Veldelta = actor->GetVelocityDelta();
		Veldelta += Scene->GetGravity();
		glm::vec3 BodyVelocity = actor->GetLinearVelocity();
		BodyVelocity += Veldelta * dt;
		actor->SetLinearVelocity(BodyVelocity);
		glm::vec3 startpos = actor->GetTransfrom()->GetPos();
		actor->GetTransfrom()->SetPos(startpos + (BodyVelocity*dt));
		actor->ResetForceThisFrame();
	}

	void TDSolver::ResolveCollisions(TDScene* scene)
	{
		for (int i = 0; i < scene->GetActors().size(); i++)
		{
			for (int j = 0; j < scene->GetActors().size(); j++)
			{

			}
		}
	}
}