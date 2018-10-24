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
	CORE_API static AISystem* Get();
	NavigationMesh* mesh = nullptr;
	static EAINavigationMode::Type GetPathMode();
	template<class T>
	static T* GetDirector()
	{
		return (T*)Get()->Director;
	}
private:
	AIDirector* Director = nullptr;
	static AISystem* Instance;
	EAINavigationMode::Type CurrentMode = EAINavigationMode::Limit;
};

