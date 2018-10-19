#include "TDPCH.h"
#include "TDSolver.h"
#include "TDScene.h"
#include "TDCollisionHandlers.h"
#define USE_LINEAR_PROJECTION 1
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
		if (Actor->GetActorType() != TDActorType::RigidStatic)
		{
			Veldelta += Scene->GetGravity();
		}
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
			for (int j = i; j < scene->GetActors().size(); j++)
			{
				TDActor* Actor = scene->GetActors()[i];
				if (scene->GetActors()[i] == scene->GetActors()[j])
				{
					continue;
				}
				ProcessCollisions(Actor->GetAttachedShapes()[0], scene->GetActors()[j]->GetAttachedShapes()[0]);
				//	ProcessCollisions(scene->GetActors()[i], scene->GetActors()[j]);
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
			const glm::vec3 RelVel = A->GetOwner()->GetLinearVelocity() - B->GetOwner()->GetLinearVelocity();
			const glm::vec3 RelNrm = glm::normalize(data.Direction);
			if (glm::dot(RelVel, RelNrm) > 0.0f)
			{
				return;//object moving away
			}
			const float invmassA = A->GetOwner()->GetInvBodyMass();
			const float invmassB = B->GetOwner()->GetInvBodyMass();
			const float InvMassSum = invmassA + invmassB;
			//Coefficient of Restitution min of both materials 
			float CoR = fminf(A->GetPhysicalMaterial()->Restitution, B->GetPhysicalMaterial()->Restitution);
			float numerator = (-(1.0f + CoR) * glm::dot(RelVel, RelNrm));			float j = numerator / InvMassSum;
			const glm::vec3 impluse = RelNrm * j;

			A->GetOwner()->SetLinearVelocity(A->GetOwner()->GetLinearVelocity() + impluse * invmassA);
			B->GetOwner()->SetLinearVelocity(B->GetOwner()->GetLinearVelocity() - impluse * invmassB);
#if USE_LINEAR_PROJECTION
			const float Slack = 0.01f;
			const float LinearProjectionPercent = 0.45f;
			float depth = fmaxf(data.depth - Slack, 0.0f);
			float scalar = depth / InvMassSum;
			const glm::vec3 Reporjections = data.Direction * scalar *LinearProjectionPercent;
			A->GetOwner()->GetTransfrom()->SetPos(A->GetOwner()->GetTransfrom()->GetPos() + Reporjections * invmassA);
			B->GetOwner()->GetTransfrom()->SetPos(B->GetOwner()->GetTransfrom()->GetPos() - Reporjections * invmassB);
#endif

		}
	}
}