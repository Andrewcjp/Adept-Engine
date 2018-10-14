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

	private:
		void ProcessCollisions(TDShape * A, TDShape * B);
		void IntergrateActor(TDActor * actor, float dt, TDScene * Scene);
	};
}

