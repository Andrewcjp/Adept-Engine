#pragma once
class NavigationMesh;
class AISystem
{
public:
	AISystem();
	~AISystem();
	static void StartUp();
	static void ShutDown();
	static AISystem* Get();
	NavigationMesh* mesh = nullptr;
private:
	static AISystem* Instance;

};

