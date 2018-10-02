#pragma once
namespace TD
{
	class TDScene;
	class TDSolver;
	class TDPhysics
	{
	public:
		static TDPhysics* CreatePhysics();
		TD_API void StartUp();
		TD_API void StartStep(float deltaTime);
		TD_API void ShutDown();
		TD_API static TDScene* GetScene();
		TD_API static TDPhysics* Get();
	private:
		TDPhysics();
		~TDPhysics();
		TDSolver* Solver = nullptr;
		TDScene* CurrentScene = nullptr;
		static TDPhysics* Instance;
	};

}