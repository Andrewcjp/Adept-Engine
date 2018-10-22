#pragma once
class NavigationMesh;
class AIDirector;
class Scene;
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
private:
	AIDirector* Director = nullptr;
	static AISystem* Instance;
};

