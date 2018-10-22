#pragma once

namespace TD
{
	class TDScene;
	class TDActor;
	class TDShape;
	class TDRigidDynamic;
	struct ContactData;
	class TDSolver
	{
	public:
		TDSolver();
		~TDSolver();
		void IntergrateScene(TDScene* scene, float dt);
		void ResolveCollisions(TDScene* scene);
		static void ProcessCollisions(TDShape * A, TDShape * B);

		
		
	private:
		static void ProcessCollisionResponse(TDRigidDynamic * A, TDRigidDynamic * B, ContactData * data, float CoR);
		void IntergrateActor(TDRigidDynamic * actor, float dt, TDScene * Scene);
	};
}

