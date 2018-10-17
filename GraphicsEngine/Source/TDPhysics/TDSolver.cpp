#include "TDPCH.h"
#include "TDSolver.h"
#include "TDScene.h"
#include "TDCollisionHandlers.h"
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

	void TDSolver::IntergrateActor(TDActor* Actor, float dt, TDScene* Scene)
	{
		glm::vec3 Veldelta = Actor->GetVelocityDelta();
		Veldelta += Scene->GetGravity();
		glm::vec3 BodyVelocity = Actor->GetLinearVelocity();
		BodyVelocity += Veldelta * dt;
		Actor->SetLinearVelocity(BodyVelocity);
		glm::vec3 startpos = Actor->GetTransfrom()->GetPos();
		Actor->GetTransfrom()->SetPos(startpos + (BodyVelocity*dt));
		Actor->ResetForceThisFrame();
	}

	void TDSolver::ResolveCollisions(TDScene* scene)
	{
		for (int i = 0; i < scene->GetActors().size(); i++)
		{
			for (int j = 0; j < scene->GetActors().size(); j++)
			{
				//	ProcessCollisions(scene->GetActors()[i], scene->GetActors()[j]);
			}
		}
	}

	void TDSolver::ProcessCollisions(TDShape* A, TDShape* B)
	{
		EShapeType::Type AType = A->GetShapeType();
		EShapeType::Type BType = B->GetShapeType();

		const bool flip = (AType > BType);
		if (flip)
		{
			std::swap(A, B);
			std::swap(AType, BType);
		}

		ContactMethod con = ContactMethodTable[AType][BType];
		DebugEnsure(con);
		ContactData data;
		con(A, B, &data);
	}
}