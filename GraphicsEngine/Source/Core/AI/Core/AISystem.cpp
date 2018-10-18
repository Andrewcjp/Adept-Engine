#include "Stdafx.h"
#include "AISystem.h"
#include "AI/Core/NavigationMesh.h"
#include "AIDirector.h"

AISystem* AISystem::Instance = nullptr;

AISystem::AISystem()
{
	mesh = new NavigationMesh();
	Director = new AIDirector();
	//mesh->GenTestMesh();
}

AISystem::~AISystem()
{}

void AISystem::SetupForScene(Scene* newscene)
{
	Director->SetScene(newscene);
}

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
