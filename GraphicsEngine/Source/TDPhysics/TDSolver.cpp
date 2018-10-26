#include "TDPCH.h"
#include "TDSolver.h"
#include "TDScene.h"
#include "TDCollisionHandlers.h"
#include "TDRigidDynamic.h"
#include "TDPhysics.h"
#include "TDSimConfig.h"
#define USE_LINEAR_PROJECTION 1
namespace TD
{
	TDSolver::TDSolver()
	{
		SolverIterations = TDPhysics::GetCurrentSimConfig()->SolverIterationCount;
	}

	TDSolver::~TDSolver()
	{}

	void TDSolver::IntergrateScene(TDScene* scene, float dt)
	{
#if !BUILD_FULLRELEASE
		TDPhysics::StartTimer(TDPerfCounters::IntergrateScene);
#endif
		for (int i = 0; i < scene->GetDynamicActors().size(); i++)
		{
			IntergrateActor(scene->GetDynamicActors()[i], dt, scene);
		}
#if !BUILD_FULLRELEASE
		TDPhysics::EndTimer(TDPerfCounters::IntergrateScene);
#endif
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
		const glm::vec3 startpos = actor->GetTransfrom()->GetPos();
		actor->GetTransfrom()->SetPos(startpos + (BodyVelocity*dt));

		glm::vec3 AVelDelta = actor->GetAngularVelocityDelta();
		glm::vec3 BodyAngVel = actor->GetAngularVelocity();
		BodyAngVel += AVelDelta * dt;
		actor->SetAngularVelocity(BodyAngVel);
		const glm::vec3 startRot = actor->GetTransfrom()->GetEulerRot();
		actor->GetTransfrom()->SetQrot(glm::quat(startRot + (BodyAngVel*dt)));
		actor->UpdateSleepTimer(dt);
		actor->ResetForceThisFrame();
	}

	void TDSolver::ResolveCollisions(TDScene* scene)
	{
#if !BUILD_FULLRELEASE
		TDPhysics::StartTimer(TDPerfCounters::ResolveCollisions);
#endif
		for (int Iterations = 0; Iterations < SolverIterations; Iterations++)
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
#if !BUILD_FULLRELEASE
		TDPhysics::EndTimer(TDPerfCounters::ResolveCollisions);
#endif
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
			ProcessCollisionResponse(TDActor::ActorCast<TDRigidDynamic>(A->GetOwner()), TDActor::ActorCast<TDRigidDynamic>(B->GetOwner()), &data, A->GetPhysicalMaterial(), B->GetPhysicalMaterial());
		}
	}

	void TDSolver::ProcessCollisionResponse(TDRigidDynamic * A, TDRigidDynamic * B, ContactData * data, const TDPhysicalMaterial* AMaterial, const TDPhysicalMaterial* BMaterial)
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
		//Coefficient of Restitution min of both materials 
		const float CoR = fminf(AMaterial->Restitution, BMaterial->Restitution);
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
		const glm::vec3 Reporjections = data->Direction * scalar * LinearProjectionPercent;
		if (A != nullptr)
		{
			A->GetTransfrom()->SetPos(A->GetTransfrom()->GetPos() + Reporjections * invmassA);
		}
		if (B != nullptr)
		{
			B->GetTransfrom()->SetPos(B->GetTransfrom()->GetPos() - Reporjections * invmassB);
		}
#endif
		//Apply Friction
		glm::vec3 tangent = (RelVel * glm::dot(RelVel, RelNrm));
		if (glm::length2(tangent) == 0)
		{
			return;// no Size to tangent
		}
		tangent = glm::normalize(tangent);
		numerator = -glm::dot(RelVel, tangent);
		float jt = numerator / InvMassSum;
		//divide by contacts
		if (jt == 0)
		{
			return;
		}
		//todo: when to use dynamic friction?
		float friction = sqrtf(AMaterial->StaticFriction * BMaterial->StaticFriction);
		if (jt > j * friction)
		{
			jt = j * friction;
		}
		else if (jt < -j * friction)
		{
			jt = -j * friction;
		}
		const glm::vec3 FrictionImpluse = tangent * jt;
		if (A != nullptr)
		{
			A->SetLinearVelocity(A->GetLinearVelocity() - FrictionImpluse * invmassA);
		}
		if (B != nullptr)
		{
			B->SetLinearVelocity(B->GetLinearVelocity() + FrictionImpluse * invmassB);
		}
	}
}