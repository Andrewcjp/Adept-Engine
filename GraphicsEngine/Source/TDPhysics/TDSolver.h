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
	struct ContactCallBack
	{
		ContactPair* Pair;
		int LastUsedTime = 0;
		bool Sent = false;
	};
	class TDSolver
	{
	public:
		TDSolver();
		~TDSolver();
		void IntergrateScene(TDScene* scene, float dt);
		void TickContacts();
		void AddContact(ShapeCollisionPair * pair);
		void ResolveCollisions(TDScene* scene);
		std::string ReportbroadPhaseStats();
		static void ProcessCollisions(ShapeCollisionPair * A);
		void ProcessResponsePair(ShapeCollisionPair * pair);
		void PostIntergrate(ShapeCollisionPair * pair);
		void ResolveConstraints(TDScene* scene);
		static void RunPostFixup(TDRigidDynamic * A, TDRigidDynamic * B, ContactData * data);
		///Returns the current time step for the current processed scene
		static float GetTimeStep();
		static TDSolver* Get();
		void FinishAccumlateForces(TDScene* scene);
	private:
		static TDSolver* Instance;
		int SolverIterations = 10;
		static void ProcessCollisionResponse(TDRigidDynamic * A, TDRigidDynamic * B, ContactData * data, const TDPhysicalMaterial * AMaterial, const TDPhysicalMaterial * BMateria, int contactindexl);
		void IntergrateActor(TDRigidDynamic * actor, float dt, TDScene * Scene);
		void ProcessBroadPhase(TDScene * scene);
		
		int BroadPhaseCount = 0;
		float CurrentTimeStep = 0.0f;
		std::vector<ActorCollisionPair> NarrowPhasePairs;
		//callbacks
		std::vector<ContactPair*> NewSimulationCallbackPairs;		
		std::vector<ContactPair*> NewSimulationTriggerCallbackPairs;
		std::vector<ContactCallBack> TriggerCallbacks;
		std::vector<ContactCallBack> ContactCallbacks;
	};
}

