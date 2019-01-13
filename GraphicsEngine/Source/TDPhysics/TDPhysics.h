#pragma once
#include "TDTypes.h"

namespace TD
{
	class TDSimulationCallbacks;
}
namespace TD
{
	class TDScene;
	class TDSolver;
	class TDSimConfig;
	class TDConstraint;
	namespace TDThreading
	{
		class TaskGraph;
	}
	class TDPhysics
	{
	public:
		/**
		*\brief Creates a Physics Engine Object for use
		*\param BuildID provided by TD_VERSION_NUMBER in TDPhysicsAPI.h
		*\param TDSimConfig Object with simulator settings in.
		*\return a valid TDPhysics* instance unless the BuildID is invalid In which case nullptr is returned;
		*/
		TD_API static TDPhysics * CreatePhysics(unsigned int BuildID, TDSimConfig* SimConfig = nullptr);
		/**
		*\brief Initializes the Simulator ready for work.
		*/
		TD_API void StartUp();
		/**
		*\brief Starts the step of the Simulator.
		*\param TimeStep for the Simulator to use
		*/
		TD_API void StartStep(TDScene* scene, float TimeStep);
		/**
		*\brief Destroy the Simulator.
		*
		*After this call any TDPhysics pointers are invalid for use.
		*/
		TD_API void ShutDown();
		/**
		*\brief Returns the current TDPhysics instance
		*
		*returns nullptr if the Simulator has not been initialized
		*/
		TD_API static TDPhysics* Get();
		/**
		* \brief Creates a new TDScene for Use.
		*/
		TD_API TDScene* CreateScene();

		TD_API TDConstraint* CreateConstraint(TDActor* BodyA, TDActor* BodyB, const ConstraintDesc& desc);
		///\brief Returns the current Config for the simulator.
		///
		///some config properties can be changed at runtime
		static TDSimConfig* GetCurrentSimConfig();

		//Internal Functions
		static void StartTimer(TDPerfCounters::Type timer);
		static void EndTimer(TDPerfCounters::Type timer);
		static void DrawDebugLine(glm::vec3 LineStart, glm::vec3 LineEnd, glm::vec3 Colour, float lifetime);
		static void DrawDebugPoint(glm::vec3 pos, glm::vec3 colour, float Lifetime);
		TDSolver* Solver = nullptr;
		static TDThreading::TaskGraph* GetTaskGraph();
		void SimulationContactCallback(std::vector<ContactPair*> & SimulationCallbackPairs);
		void TriggerSimulationContactCallback(std::vector<ContactPair*> & SimulationCallbackPairs);
		TD_API void SetDebugShowBroadPhaseShapes(bool state);
		TD_API void SetDebugShowContacts(bool state);
	private:
		TDPhysics();
		~TDPhysics();
		TDThreading::TaskGraph* TDTaskGraph = nullptr;

		std::vector<TDScene*> Scenes;
		static TDPhysics* Instance;
		TDSimConfig* CurrentSimConfig = nullptr;
		TDSimulationCallbacks* Callbacks = nullptr;

	};

}