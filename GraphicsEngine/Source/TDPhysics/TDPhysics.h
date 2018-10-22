#pragma once
namespace TD
{
	class TDScene;
	class TDSolver;
	class TDOptions;
	class TDPhysics
	{
	public:
		/**
		*\brief Creates a Physics Engine Object for use
		*\param BuildID provided by TD_VERSION_NUMBER in TDPhysicsAPI.h
		*/
		TD_API static TDPhysics * CreatePhysics(unsigned int BuildID);
		/**
		*\brief Initializes the Simulator ready for work
		*/
		TD_API void StartUp();
		/**
		*\brief Starts the step of the Simulator 
		*\param TimeStep for the Simulator to use
		*/
		TD_API void StartStep(float TimeStep);
		/**
		*\brief Destroy the Simulator
		*After this call any TDPhysics pointers are invalid for use.
		*/
		TD_API void ShutDown();
		/**
		*\brief Returns the current TDPhysics instance, nullptr if the Simulator has not been initialized 
		*/
		TD_API static TDPhysics* Get();
		/**
		*\brief Creates a new TDScene for Use.
		*/
		TD_API TDScene* CreateScene();

		static TDOptions* GetCurrentOptions();
	private:
		TDPhysics();
		~TDPhysics();

		TDSolver* Solver = nullptr;
		std::vector<TDScene*> Scenes;
		static TDPhysics* Instance;
		TDOptions* CurrentOptions = nullptr;
		
	};

}