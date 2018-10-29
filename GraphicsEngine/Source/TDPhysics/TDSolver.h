#pragma once
namespace TD
{
	class TDScene;
	class TDActor;
	class TDShape;
	class TDRigidDynamic;
	struct ContactData;
	struct TDPhysicalMaterial;
	class TDSolver
	{
	public:
		TDSolver();
		~TDSolver();
		void IntergrateScene(TDScene* scene, float dt);
		void ResolveCollisions(TDScene* scene);
		bool BroadPhaseTest(TDActor * A, TDActor * B);
		std::string ReportbroadPhaseStats();
		static void ProcessCollisions(TDShape * A, TDShape * B);
	private:
		int SolverIterations = 5;
		static void ProcessCollisionResponse(TDRigidDynamic * A, TDRigidDynamic * B, ContactData * data, const TDPhysicalMaterial * AMaterial, const TDPhysicalMaterial * BMaterial);
		void IntergrateActor(TDRigidDynamic * actor, float dt, TDScene * Scene);
		int BroadPhaseCount = 0;
	};
}

