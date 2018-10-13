#include "Stdafx.h"
#include "AISystem.h"
#include "AI/Core/NavigationMesh.h"
AISystem* AISystem::Instance = nullptr;

AISystem::AISystem()
{
	mesh = new NavigationMesh();
	//mesh->GenTestMesh();
}


AISystem::~AISystem()
{}

void AISystem::StartUp()
{
	Instance = new AISystem();
}

void AISystem::ShutDown()
{
	SafeDelete(Instance);
}

AISystem * AISystem::Get()
{
	return Instance;
}
