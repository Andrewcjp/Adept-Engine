#include "TDSolver.h"
#include "Constraints/TDConstraint.h"
#include "TDBroadphase.h"
#include "TDCollisionHandlers.h"
#include "TDPhysics.h"
#include "TDRigidDynamic.h"
#include "TDScene.h"
#include "TDSimConfig.h"
#include <sstream>
#define USE_LINEAR_PROJECTION 1
#define USE_THREADED_COLLISION_DETECTION 0

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
		SimulationCallbackPairs.clear();
		for (int i = 0; i < NarrowPhasePairs.size(); i++)
		{
			PostIntergrate(&NarrowPhasePairs[i]);
			AddContact(&NarrowPhasePairs[i]);
			NarrowPhasePairs[i].data.Reset();
		}
		TDPhysics::Get()->SimulationContactCallback(SimulationCallbackPairs);
#if !BUILD_FULLRELEASE
		TDPhysics::EndTimer(TDPerfCounters::IntergrateScene);
#endif
	}
	void TDSolver::AddContact(CollisionPair*pair)
	{
		if (!pair->IsNew)
		{
			return;
		}
		pair->IsNew = false;
		SimulationCallbackPairs.push_back(new ContactPair(pair->first->GetAttachedShapes()[0], pair->second->GetAttachedShapes()[0]));
	}
	void TDSolver::IntergrateActor(TDRigidDynamic * actor, float dt, TDScene * Scene)
	{
		if (actor->IsBodyAsleep())
		{
			actor->UpdateSleepTimer(dt);
			return;
		}
		glm::vec3 Veldelta = actor->GetLinearVelocityDelta() / actor->GetInvBodyMass();
		if (actor->IsAffectedByGravity())
		{
			Veldelta += Scene->GetGravity();
		}
		glm::vec3 BodyVelocity = actor->GetLinearVelocity();
		BodyVelocity += Veldelta * dt;
		actor->SetLinearVelocity(BodyVelocity);
		const glm::vec3 startpos = actor->GetTransfrom()->GetPos();
		actor->GetTransfrom()->SetPos(startpos + (BodyVelocity*dt));

		glm::vec3 AVelDelta = glm::vec4(actor->GetAngularVelocityDelta(), 0.0f)*actor->GetInertiaTensor();
		glm::vec3 BodyAngVel = actor->GetAngularVelocity();
		BodyAngVel += AVelDelta * dt;

#if 0		
		const glm::vec3 startRot = actor->GetTransfrom()->GetEulerRot();
		actor->GetTransfrom()->SetQrot(glm::quat(startRot + (BodyAngVel*dt)));
#else
		float w = glm::length2(BodyAngVel);
		// Integrate the rotation using closed form quaternion integrator
		if (w != 0.0f)//need to protect the sqrt
		{
			w = glm::sqrt(w);
			float maxW = 1e+7f;
			if (w > maxW)
			{
				BodyAngVel = glm::normalize(BodyAngVel)* maxW;
				w = maxW;
			}
			float v = dt * w * 0.5f;//in rads!
			float s = glm::sin(v);
			float q = glm::cos(v);
			s /= w;

			const glm::vec3 pqr = BodyAngVel * s;
			glm::quat quatvel = glm::quat(pqr.x, pqr.y, pqr.z, 0.0f);
			glm::quat result = quatvel * actor->GetTransfrom()->GetQuatRot();
			result += actor->GetTransfrom()->GetQuatRot() * q;
			actor->GetTransfrom()->SetQrot(glm::normalize(result));
		}
#endif
		actor->SetAngularVelocity(BodyAngVel);
		actor->UpdateSleepTimer(dt);
		actor->ResetForceThisFrame();
	}
#define USE_PHASE 1
	void TDSolver::ProcessBroadPhase(TDScene* scene)
	{
		BroadPhaseCount = 0;
#if 0
		NarrowPhasePairs.clear();
		for (int i = 0; i < scene->GetActors().size(); i++)
		{
			for (int j = i; j < scene->GetActors().size(); j++)
			{
				TDActor* Actor = scene->GetActors()[i];
				TDActor* Actorb = scene->GetActors()[j];
				if (scene->GetActors()[i] == scene->GetActors()[j])
				{
					continue;
				}
				NarrowPhasePairs.push_back(CollisionPair(Actor, Actorb));
			}
		}
#else
		scene->UpdateBroadPhase();
		NarrowPhasePairs = scene->GetPairs();
#endif
		//printf(ReportbroadPhaseStats().c_str());
	}

	void TDSolver::ResolveCollisions(TDScene* scene)
	{
#if !BUILD_FULLRELEASE
		TDPhysics::StartTimer(TDPerfCounters::ResolveCollisions);
#endif
		ProcessBroadPhase(scene);
		for (int i = 0; i < NarrowPhasePairs.size(); i++)
		{
			NarrowPhasePairs[i].data.Reset();//reset before check collisions again
			ProcessCollisions(&NarrowPhasePairs[i]);
		}
		for (int i = 0; i < NarrowPhasePairs.size(); i++)
		{
			NarrowPhasePairs[i].first->ComputeKE();
			NarrowPhasePairs[i].second->ComputeKE();
		}
		for (int Iterations = 0; Iterations < SolverIterations; Iterations++)
		{

#if USE_THREADED_COLLISION_DETECTION
			const int BatchSize = 50;
			std::function <void(int)> ProcessCollisionsFunc = [&](int threadIndex)
			{
				const int StartingIndex = threadIndex * BatchSize;
				int ThisBatchcount = std::min(StartingIndex + BatchSize, (int)NarrowPhasePairs.size());
				for (int i = StartingIndex; i < ThisBatchcount; i++)
				{
					ProcessCollisions(&NarrowPhasePairs[i]);
				}
			};
			const int threadcount = std::min(TDPhysics::GetTaskGraph()->GetThreadCount(), (int)NarrowPhasePairs.size() / BatchSize + 1);
			TDPhysics::GetTaskGraph()->RunTaskOnGraph(ProcessCollisionsFunc, threadcount);
#else
			for (int i = 0; i < NarrowPhasePairs.size(); i++)
			{
				NarrowPhasePairs[i].first->ComputeKE();
				NarrowPhasePairs[i].second->ComputeKE();
				ProcessResponsePair(&NarrowPhasePairs[i]);
			}
#endif

		}
		for (int i = 0; i < NarrowPhasePairs.size(); i++)
		{
			NarrowPhasePairs[i].first->ValidateKE();
			NarrowPhasePairs[i].second->ValidateKE();
		}
#if !BUILD_FULLRELEASE
		TDPhysics::EndTimer(TDPerfCounters::ResolveCollisions);
#endif
	}

	std::string TDSolver::ReportbroadPhaseStats()
	{
		std::stringstream ss;
		ss << "removed " << BroadPhaseCount << " intersections\n";
		return ss.str();
	}

	void TDSolver::ProcessCollisions(CollisionPair * pair)
	{
		TDShape* A = pair->first->GetAttachedShapes()[0];
		TDShape* B = pair->second->GetAttachedShapes()[0];
		TDShapeType::Type AType = A->GetShapeType();
		TDShapeType::Type BType = B->GetShapeType();

		const bool flip = (AType > BType);
		if (flip)
		{
			std::swap(pair->first, pair->second);
			std::swap(A, B);
			std::swap(AType, BType);
		}
		if (!A->GetFlags().GetFlagValue(TDShapeFlags::ESimulation) || !B->GetFlags().GetFlagValue(TDShapeFlags::ESimulation))
		{
			return;
		}
		ContactMethod con = ContactMethodTable[AType][BType];
		DebugEnsure(con);
		con(A, B, &pair->data);
	}

	void TDSolver::ProcessResponsePair(CollisionPair* pair)
	{
		TDShape* A = pair->first->GetAttachedShapes()[0];
		TDShape* B = pair->second->GetAttachedShapes()[0];
		if (pair->data.Blocking)
		{
			int depthest = 0;
			float current = 0.0;
			for (int i = 0; i < pair->data.ContactCount; i++)
			{
				if (current < pair->data.depth[i])
				{
					current = pair->data.depth[i];
					depthest = i;
				}
			}
			for (int i = 0; i < pair->data.ContactCount; i++)
			{
				ProcessCollisionResponse(TDActor::ActorCast<TDRigidDynamic>(A->GetOwner()), TDActor::ActorCast<TDRigidDynamic>(B->GetOwner()),
					&pair->data, A->GetPhysicalMaterial(), B->GetPhysicalMaterial(), i);
			}
		}
	}
	void TDSolver::PostIntergrate(CollisionPair* pair)
	{
		TDShape* A = pair->first->GetAttachedShapes()[0];
		TDShape* B = pair->second->GetAttachedShapes()[0];
		RunPostFixup(TDActor::ActorCast<TDRigidDynamic>(A->GetOwner()), TDActor::ActorCast<TDRigidDynamic>(B->GetOwner()), &pair->data);
	}

	void TDSolver::ResolveConstraints(TDScene * scene)
	{
		for (int i = 0; i < scene->GetConstraints().size(); i++)
		{
			scene->GetConstraints()[i]->Resolve();
		}
	}

	void TDSolver::RunPostFixup(TDRigidDynamic * A, TDRigidDynamic * B, ContactData * data)
	{
		///todo:: this is broke- write a unit test?
		if (data->ContactCount == 0)
		{
			return;
		}
		//return;
		glm::vec3 AVG;
		for (int i = 0; i < data->ContactCount; i++)
		{
			AVG += data->Direction[i];
		}
		AVG /= data->ContactCount;

		float AVGdepth = 0.0f;
		for (int i = 0; i < data->ContactCount; i++)
		{
			AVGdepth += data->depth[i];
		}
		AVGdepth /= data->ContactCount;

		for (int i = 0; i < data->ContactCount; i++)
			//int i = 0;
		{
			TDPhysics::DrawDebugPoint(data->ContactPoints[i], AVG, 0.0f);
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
			float InvMassSum = invmassA + invmassB;
#if USE_LINEAR_PROJECTION
			const float Slack = 0.1f;
			const float LinearProjectionPercent = 0.45f;
			float depth = fmaxf(data->depth[i] - Slack, 0.0f);
			float scalar = depth / InvMassSum;
			const glm::vec3 Reporjections = data->Direction[i] * scalar * LinearProjectionPercent;
			if (A != nullptr)
			{
				A->GetTransfrom()->SetPos(A->GetTransfrom()->GetPos() + (Reporjections * invmassA) / data->ContactCount);
			}
			if (B != nullptr)
			{
				B->GetTransfrom()->SetPos(B->GetTransfrom()->GetPos() - (Reporjections * invmassB) / data->ContactCount);
			}
#endif
		}
	}

	void TDSolver::ProcessCollisionResponse(TDRigidDynamic * A, TDRigidDynamic * B, ContactData * data, const TDPhysicalMaterial * AMaterial, const TDPhysicalMaterial * BMaterial, int contactindex)
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
		const glm::vec3 RelNrm = glm::normalize(data->Direction[contactindex]);
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
		if (data->ContactCount > 0 && j != 0.0f)
		{
			j /= (float)data->ContactCount;
		}
		const glm::vec3 impluse = RelNrm * j;
		if (A != nullptr)
		{
			A->SetLinearVelocity(A->GetLinearVelocity() + impluse * invmassA);
		}
		if (B != nullptr)
		{
			B->SetLinearVelocity(B->GetLinearVelocity() - impluse * invmassB);
		}
		//Apply Friction
		glm::vec3 tangent = RelVel - (RelNrm * glm::dot(RelVel, RelNrm));
		if (glm::length2(tangent) == 0)
		{
			return;// no Size to tangent
		}
		tangent = glm::normalize(tangent);
		numerator = -glm::dot(RelVel, tangent);
		float jt = numerator / InvMassSum;
		if (data->ContactCount > 0 && jt != 0.0f)
		{
			jt /= (float)data->ContactCount;
		}
		
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
			A->SetLinearVelocity(A->GetLinearVelocity() + FrictionImpluse * invmassA);
		}
		if (B != nullptr)
		{
			B->SetLinearVelocity(B->GetLinearVelocity() - FrictionImpluse * invmassB);
		}
	}
}