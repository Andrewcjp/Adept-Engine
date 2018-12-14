#pragma once
#include "TDTypes.h"

namespace TD
{
	class TDScene;
	class TDActor;
	class TDShape;
	class TDRigidDynamic;
	struct ContactData;
	struct TDPhysicalMaterial;
	class TDBroadphase;
	class TDSolver
	{
	public:
		TDSolver();
		~TDSolver();
		void IntergrateScene(TDScene* scene, float dt);
		void AddContact(CollisionPair * pair);
		void ResolveCollisions(TDScene* scene);
		std::string ReportbroadPhaseStats();
		static void ProcessCollisions(CollisionPair * A);
		void ProcessResponsePair(CollisionPair * pair);
		void PostIntergrate(CollisionPair * pair);
		void ResolveConstraints(TDScene* scene);
		static void RunPostFixup(TDRigidDynamic * A, TDRigidDynamic * B, ContactData * data);
		///Returns the current time step for the current processed scene
		static float GetTimeStep();
		static TDSolver* Get();
	private:
		static TDSolver* Instance;
		int SolverIterations = 5;
		static void ProcessCollisionResponse(TDRigidDynamic * A, TDRigidDynamic * B, ContactData * data, const TDPhysicalMaterial * AMaterial, const TDPhysicalMaterial * BMateria, int contactindexl);
		void IntergrateActor(TDRigidDynamic * actor, float dt, TDScene * Scene);
		void ProcessBroadPhase(TDScene * scene);
		int BroadPhaseCount = 0;
		float CurrentTimeStep = 0.0f;
		std::vector<CollisionPair> NarrowPhasePairs;
		std::vector<ContactPair*> SimulationCallbackPairs;
		std::vector<ContactPair*> SimulationTriggerCallbackPairs;
		std::vector<ContactPair*> OldSimulationCallbackPairs;
	};
}

