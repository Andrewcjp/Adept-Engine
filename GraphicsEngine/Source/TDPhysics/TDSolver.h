#pragma once

namespace TD
{
	class TDScene;
	class TDActor;
	class TDSolver
	{
	public:
		TDSolver();
		~TDSolver();
		void IntergrateScene(TDScene* scene,float dt);
		void ResolveCollisions(TDScene* scene);
	private:
		void IntergrateActor(TDActor * actor, float dt, TDScene * Scene);
	};
}

