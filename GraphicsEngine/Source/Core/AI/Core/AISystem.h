#pragma once
class NavigationMesh;
class AIDirector;
class Scene;
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
class AISystem
{
public:
	AISystem();
	~AISystem();
	void SetupForScene(Scene * newscene);
	static void StartUp();
	static void ShutDown();
	static AISystem* Get();
	NavigationMesh* mesh = nullptr;
	static EAINavigationMode::Type GetPathMode();
private:
	AIDirector* Director = nullptr;
	static AISystem* Instance;
	EAINavigationMode::Type CurrentMode = EAINavigationMode::Limit;
};

