#pragma once

namespace TD
{
	class TDScene;
	class TDActor;
	class TDShape;
	class TDSolver
	{
	public:
		TDSolver();
		~TDSolver();
		void IntergrateScene(TDScene* scene, float dt);
		void ResolveCollisions(TDScene* scene);
		static void ProcessCollisions(TDShape * A, TDShape * B);
	private:
		
		void IntergrateActor(TDActor * actor, float dt, TDScene * Scene);
	};
}

