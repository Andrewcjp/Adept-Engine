#include "TDSolver.h"
#include "Constraints/TDConstraint.h"
#include "TDBroadphase.h"
#include "TDCollisionHandlers.h"
#include "TDPhysics.h"
#include "TDRigidDynamic.h"
#include "TDScene.h"
#include "TDSimConfig.h"
#include <sstream>
#include "Utils/VectorUtils.h"
#define USE_LINEAR_PROJECTION 1
#define USE_THREADED_COLLISION_DETECTION 0

namespace TD
{
	TDSolver* TDSolver::Instance = nullptr;
	TDSolver::TDSolver()
	{
		Instance = this;
		SolverIterations = TDPhysics::GetCurrentSimConfig()->SolverIterationCount;
	}

	TDSolver::~TDSolver()
	{}

	void TDSolver::IntergrateScene(TDScene* scene, float dt)
	{
		CurrentTimeStep = dt;
#if !BUILD_FULLRELEASE
		TDPhysics::StartTimer(TDPerfCounters::IntergrateScene);
#endif
		for (int i = 0; i < scene->GetDynamicActors().size(); i++)
		{
			IntergrateActor(scene->GetDynamicActors()[i], dt, scene);
		}

		for (int i = 0; i < NarrowPhasePairs.size(); i++)
		{
			for (int p = 0; p < NarrowPhasePairs[i].ShapePairs.size(); p++)
			{
				PostIntergrate(&(NarrowPhasePairs[i].ShapePairs[p]));
				AddContact(&(NarrowPhasePairs[i].ShapePairs[p]));
			}
			NarrowPhasePairs[i].Reset();
		}
		TDPhysics::Get()->SimulationContactCallback(NewSimulationCallbackPairs);
		TDPhysics::Get()->TriggerSimulationContactCallback(NewSimulationTriggerCallbackPairs);
		NewSimulationCallbackPairs.clear();
		NewSimulationTriggerCallbackPairs.clear();
		TickContacts();
#if !BUILD_FULLRELEASE
		TDPhysics::EndTimer(TDPerfCounters::IntergrateScene);
#endif
	}

	void TDSolver::TickContacts()
	{
		const int Expire = TDPhysics::GetCurrentSimConfig()->ContactExpireFrameCount;
		for (int i = 0; i < TriggerCallbacks.size(); i++)
		{
			TriggerCallbacks[i].LastUsedTime++;
			if (TriggerCallbacks[i].LastUsedTime > Expire)
			{
				TriggerCallbacks.erase(TriggerCallbacks.begin() + i);
			}
		}
		for (int i = 0; i < ContactCallbacks.size(); i++)
		{
			ContactCallbacks[i].LastUsedTime++;
			if (ContactCallbacks[i].LastUsedTime > Expire)
			{
				ContactCallbacks.erase(ContactCallbacks.begin() + i);
			}
		}
	}

	void TDSolver::AddContact(ShapeCollisionPair * pair)
	{
		if (!pair->Data.Blocking)
		{
			return;
		}
		if (!pair->IsTriggerPair && !pair->SimPair)
		{
			return;
		}
		if (pair->IsTriggerPair)
		{
			if (!pair->IsPairValidForTrigger())
			{
				return;
			}
		}
		ContactPair* newpair = new ContactPair(pair->A, pair->B);
		if (pair->IsTriggerPair)
		{
			for (int i = 0; i < TriggerCallbacks.size(); i++)
			{
				if (*TriggerCallbacks[i].Pair == *newpair)
				{
					TriggerCallbacks[i].LastUsedTime = 0;
					SafeDelete(newpair);
					return;
				}
			}
			ContactCallBack callback;
			callback.Pair = newpair;
			callback.LastUsedTime = 0;
			TriggerCallbacks.push_back(callback);
			NewSimulationTriggerCallbackPairs.push_back(newpair);
		}
		else
		{
			for (int i = 0; i < ContactCallbacks.size(); i++)
			{
				if (*ContactCallbacks[i].Pair == *newpair)
				{
					ContactCallbacks[i].LastUsedTime = 0;
					SafeDelete(newpair);
					return;
				}
			}
			ContactCallBack callback;
			callback.Pair = newpair;
			callback.LastUsedTime = 0;
			ContactCallbacks.push_back(callback);
			NewSimulationCallbackPairs.push_back(newpair);
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
		glm::vec3 BodyVelocity = actor->GetLinearVelocity();
		BodyVelocity += Veldelta * dt;
		float DampingDT = actor->GetLinearDamping() *dt;
		BodyVelocity *= (1.0 - DampingDT);
		actor->SetLinearVelocity(BodyVelocity);
		const glm::vec3 startpos = actor->GetTransfrom()->GetPos();
		actor->GetTransfrom()->SetPos(startpos + (BodyVelocity*dt));

		glm::vec3 AVelDelta = glm::vec4(actor->GetAngularVelocityDelta(), 0.0f)*actor->GetInertiaTensor();
		glm::vec3 BodyAngVel = actor->GetAngularVelocity();
		BodyAngVel += AVelDelta * dt;
		DampingDT = actor->GetLinearDamping() *dt;
		BodyAngVel *= (1.0 - DampingDT);
#if 1		
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
		actor->UpdateTransfrom();
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
	void TDSolver::FinishAccumlateForces(TDScene* scene)
	{
		for (int i = 0; i < scene->GetDynamicActors().size(); i++)
		{
			TDRigidDynamic* actor = scene->GetDynamicActors()[i];
			if (actor->IsAffectedByGravity())
			{
				actor->AddForce(scene->GetGravity(), TDForceMode::AsAcceleration);
			}
		}
	}
	void TDSolver::ResolveCollisions(TDScene* scene)
	{
#if !BUILD_FULLRELEASE
		TDPhysics::StartTimer(TDPerfCounters::ResolveCollisions);
#endif
		ProcessBroadPhase(scene);
		//		DebugEnsure(NarrowPhasePairs.size());
		for (int i = 0; i < NarrowPhasePairs.size(); i++)
		{
			NarrowPhasePairs[i].Reset();//reset before check collisions again
			for (int p = 0; p < NarrowPhasePairs[i].ShapePairs.size(); p++)
			{
				ProcessCollisions(&(NarrowPhasePairs[i].ShapePairs[p]));
			}
		}
#if VALIDATE_KE
		for (int i = 0; i < NarrowPhasePairs.size(); i++)
		{
			NarrowPhasePairs[i].first->ComputeKE();
			NarrowPhasePairs[i].second->ComputeKE();//todo: crash here
		}
#endif
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
				//#if VALIDATE_KE
				//				NarrowPhasePairs[i].first->ComputeKE();
				//				NarrowPhasePairs[i].second->ComputeKE();
				//#endif
				for (int p = 0; p < NarrowPhasePairs[i].ShapePairs.size(); p++)
				{
					ProcessResponsePair(&(NarrowPhasePairs[i].ShapePairs[p]));
				}
			}
#endif
		}
#if VALIDATE_KE
		for (int i = 0; i < NarrowPhasePairs.size(); i++)
		{
			NarrowPhasePairs[i].first->ValidateKE();
			NarrowPhasePairs[i].second->ValidateKE();
		}
#endif
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

	void TDSolver::ProcessCollisions(ShapeCollisionPair * Pair)
	{
		if (!Pair->IsTriggerPair && !Pair->SimPair)
		{
			return;
		}
		TDShape* A = Pair->A;
		TDShape* B = Pair->B;
		TDShapeType::Type AType = A->GetShapeType();
		TDShapeType::Type BType = B->GetShapeType();

		const bool flip = (AType > BType);
		if (flip)
		{
			std::swap(Pair->A, Pair->B);
			std::swap(A, B);
			std::swap(AType, BType);
		}

		ContactMethod con = ContactMethodTable[AType][BType];
		DebugEnsure(con);
		con(A, B, &Pair->Data);
		if (!Pair->Data.Blocking && AType == BType && AType == TDShapeType::eSPHERE)
		{
			float t = 0;
		}
	}

	void TDSolver::ProcessResponsePair(ShapeCollisionPair * pair)
	{
		if (pair->IsTriggerPair || !pair->SimPair)
		{
			return;
		}
		TDShape* A = pair->A;
		TDShape* B = pair->B;
		if (pair->Data.Blocking)
		{
			for (int i = 0; i < pair->Data.ContactCount; i++)
			{
				ProcessCollisionResponse(TDActor::ActorCast<TDRigidDynamic>(A->GetOwner()), TDActor::ActorCast<TDRigidDynamic>(B->GetOwner()),
					&pair->Data, A->GetPhysicalMaterial(), B->GetPhysicalMaterial(), i);
			}
		}
	}
	void TDSolver::PostIntergrate(ShapeCollisionPair * pair)
	{
		if (pair->IsTriggerPair || !pair->SimPair)
		{
			return;
		}
		TDShape* A = pair->A;
		TDShape* B = pair->B;
		RunPostFixup(TDActor::ActorCast<TDRigidDynamic>(A->GetOwner()), TDActor::ActorCast<TDRigidDynamic>(B->GetOwner()), &pair->Data);
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
		if (data->ContactCount == 0)
		{
			return;
		}

		for (int i = 0; i < data->ContactCount; i++)
		{
			TDPhysics::DrawDebugPoint(data->ContactPoints[i], data->Direction[i], 0.0f);
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
			const float Slack = 0.01f;
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

	float TDSolver::GetTimeStep()
	{
		if (Get())
		{
			return Get()->CurrentTimeStep;
		}
		return 0.0f;
	}

	TD::TDSolver* TDSolver::Get()
	{
		return Instance;
	}
#define USE_ROT 0
	void TDSolver::ProcessCollisionResponse(TDRigidDynamic * A, TDRigidDynamic * B, ContactData * data, const TDPhysicalMaterial * AMaterial, const TDPhysicalMaterial * BMaterial, int contactindex)
	{
		glm::vec3 RelVel = glm::vec3(0, 0, 0);
		glm::vec3 r1;
		glm::vec3 r2;
		glm::mat4 i1;
		glm::mat4 i2;

#if USE_ROT
		if (A != nullptr)
		{
			r1 = data->ContactPoints[contactindex] - A->GetTransfrom()->GetPos();
			i1 = A->GetInertiaTensor();

		}
		if (B != nullptr)
		{
			r2 = data->ContactPoints[contactindex] - B->GetTransfrom()->GetPos();
			i2 = B->GetInertiaTensor();
		}
#endif
		if (A != nullptr && B != nullptr)
		{
#if USE_ROT
			RelVel = (B->GetLinearVelocity() + glm::cross(B->GetAngularVelocity(), r2)) - (A->GetLinearVelocity() + glm::cross(A->GetAngularVelocity(), r1));
#else
			RelVel = A->GetLinearVelocity() - B->GetLinearVelocity();
#endif
		}
		else if (B != nullptr)
		{
#if USE_ROT
			RelVel = B->GetLinearVelocity() + glm::cross(B->GetAngularVelocity(), r2);
#else
			RelVel = B->GetLinearVelocity();
#endif

		}
		else if (A != nullptr)
		{
#if USE_ROT
			RelVel = A->GetLinearVelocity() + glm::cross(A->GetAngularVelocity(), r1);
#else
			RelVel = A->GetLinearVelocity();
#endif
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

		glm::vec3 d2 = glm::vec3(0);
		glm::vec3 d3 = glm::vec3(0);
#if USE_ROT
		if (A != nullptr)
		{
			d2 = glm::cross(glm::vec3(i1 * glm::vec4(glm::cross(r1, RelNrm), 0.0f)), r1);
		}
		if (B != nullptr)
		{
			d3 = glm::cross(glm::vec3(i2 * glm::vec4(glm::cross(r2, RelNrm), 0.0f)), r2);
		}
		float denominator = InvMassSum + glm::dot(RelNrm, d2 + d3);
#else
		float denominator = InvMassSum;
#endif

		float j = numerator / denominator;
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
#if USE_ROT
		if (A != nullptr)
		{
			d2 = glm::cross(glm::vec3(i1 * glm::vec4(glm::cross(r1, tangent), 0.0f)), r1);
		}
		if (B != nullptr)
		{
			d3 = glm::cross(glm::vec3(i2 * glm::vec4(glm::cross(r2, tangent), 0.0f)), r2);
		}
		denominator = InvMassSum + glm::dot(tangent, d2 + d3);
#else
		denominator = InvMassSum;
#endif
		float jt = numerator / denominator;
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
#if USE_ROT
		if (A != nullptr)
		{
			A->SetAngularVelocity(A->GetAngularVelocity() + glm::vec3(i1 * glm::vec4(glm::cross(r1, FrictionImpluse), 0.0f)));
		}
		if (B != nullptr)
		{
			B->SetAngularVelocity(B->GetAngularVelocity() - glm::vec3(i2 * glm::vec4(glm::cross(r2, FrictionImpluse), 0.0f)));
		}
#endif
	}
}