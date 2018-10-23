#include "TDPCH.h"
#include "TDSolver.h"
#include "TDScene.h"
#include "TDCollisionHandlers.h"
#include "TDRigidDynamic.h"
#define USE_LINEAR_PROJECTION 1
namespace TD
{
	TDSolver::TDSolver()
	{}

	TDSolver::~TDSolver()
	{}

	void TDSolver::IntergrateScene(TDScene* scene, float dt)
	{
		for (int i = 0; i < scene->GetDynamicActors().size(); i++)
		{
			IntergrateActor(scene->GetDynamicActors()[i], dt, scene);
		}
	}

	void TDSolver::IntergrateActor(TDRigidDynamic * actor, float dt, TDScene * Scene)
	{
		if (actor->IsBodyAsleep())
		{
			actor->UpdateSleepTimer(dt);
			return;
		}
		glm::vec3 Veldelta = actor->GetLinearVelocityDelta();
		Veldelta += Scene->GetGravity();
		glm::vec3 BodyVelocity = actor->GetLinearVelocity();
		BodyVelocity += Veldelta * dt;
		actor->SetLinearVelocity(BodyVelocity);
		glm::vec3 startpos = actor->GetTransfrom()->GetPos();
		actor->GetTransfrom()->SetPos(startpos + (BodyVelocity*dt));
		actor->UpdateSleepTimer(dt);
		actor->ResetForceThisFrame();
	}

	void TDSolver::ResolveCollisions(TDScene* scene)
	{
		for (int i = 0; i < scene->GetActors().size(); i++)
		{
			for (int j = i; j < scene->GetActors().size(); j++)
			{
				TDActor* Actor = scene->GetActors()[i];
				if (scene->GetActors()[i] == scene->GetActors()[j])
				{
					continue;
				}
				ProcessCollisions(Actor->GetAttachedShapes()[0], scene->GetActors()[j]->GetAttachedShapes()[0]);
			}
		}
	}

	void TDSolver::ProcessCollisions(TDShape* A, TDShape* B)
	{
		TDShapeType::Type AType = A->GetShapeType();
		TDShapeType::Type BType = B->GetShapeType();

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
		if (data.Blocking)
		{
			//Coefficient of Restitution min of both materials 
			const float CoR = fminf(A->GetPhysicalMaterial()->Restitution, B->GetPhysicalMaterial()->Restitution);
			ProcessCollisionResponse(TDActor::ActorCast<TDRigidDynamic>(A->GetOwner()), TDActor::ActorCast<TDRigidDynamic>(B->GetOwner()), &data, CoR);
		}
	}

	void TDSolver::ProcessCollisionResponse(TDRigidDynamic * A, TDRigidDynamic * B, ContactData * data, float CoR)
	{
		glm::vec3 RelVel = glm::vec3(0, 0, 0);
		if (A != nullptr && B != nullptr)
		{
			RelVel = A->GetLinearVelocity() - B->GetLinearVelocity();
		}
		else if (B != nullptr)
		{
			RelVel = B->GetLinearVelocity();
		}
		else if (A != nullptr)
		{
			RelVel = A->GetLinearVelocity();
		}
		const glm::vec3 RelNrm = glm::normalize(data->Direction);
		if (glm::dot(RelVel, RelNrm) > 0.0f)
		{
			return;//object moving away
		}
		float invmassA = 0.0f;
		if (A != nullptr)
		{
			invmassA = A->GetInvBodyMass();
		}
		float invmassB = 0.0f;
		if (B != nullptr)
		{
			invmassB = B->GetInvBodyMass();
		}
		const float InvMassSum = invmassA + invmassB;
		float numerator = (-(1.0f + CoR) * glm::dot(RelVel, RelNrm));
		float j = numerator / InvMassSum;
		const glm::vec3 impluse = RelNrm * j;
		if (A != nullptr)
		{
			A->SetLinearVelocity(A->GetLinearVelocity() + impluse * invmassA);
		}
		if (B != nullptr)
		{
			B->SetLinearVelocity(B->GetLinearVelocity() - impluse * invmassB);
		}
#if USE_LINEAR_PROJECTION
		const float Slack = 0.01f;
		const float LinearProjectionPercent = 0.45f;
		float depth = fmaxf(data->depth - Slack, 0.0f);
		float scalar = depth / InvMassSum;
		const glm::vec3 Reporjections = data->Direction * scalar *LinearProjectionPercent;
		if (A != nullptr)
		{
			A->GetTransfrom()->SetPos(A->GetTransfrom()->GetPos() + Reporjections * invmassA);
		}
		if (B != nullptr)
		{
			B->GetTransfrom()->SetPos(B->GetTransfrom()->GetPos() - Reporjections * invmassB);
		}
#endif
	}
}