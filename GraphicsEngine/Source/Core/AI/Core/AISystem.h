#pragma once
#include "Navigation/NavigationMesh.h"
class NavigationMesh;
class AIDirector;
class Scene;
class BehaviourTreeManager;
class NavMeshGenerator;
namespace EAINavigationMode
{
	enum Type 
	{
		AStar,
		DStarLTE,
		DStarBoardPhase,
		Limit
	};
}
struct NavigationPath;
class AISystem
{
public:
	AISystem();
	~AISystem();
	void SetupForScene(Scene * newscene);
	static void StartUp();
	static void ShutDown();
	void Tick(float dt);
	CORE_API static AISystem* Get();
	NavigationMesh* mesh = nullptr;
	static EAINavigationMode::Type GetPathMode();
	ENavRequestStatus::Type CalculatePath(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath ** outpath);
	template<class T>
	static T* GetDirector()
	{
		return (T*)Get()->Director;
	}
	BehaviourTreeManager* GetBTManager()
	{
		return BTManager;
	};
	void GenerateMesh();
private:
	NavMeshGenerator* n = nullptr;
	BehaviourTreeManager* BTManager = nullptr; 
	AIDirector* Director = nullptr;
	static AISystem* Instance;
	EAINavigationMode::Type CurrentMode = EAINavigationMode::Limit;
};

