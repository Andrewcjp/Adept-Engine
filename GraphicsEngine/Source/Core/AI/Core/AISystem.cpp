#include "Stdafx.h"
#include "AISystem.h"
#include "AI/Core/NavigationMesh.h"
#include "AIDirector.h"
#include "core/Engine.h"
#include "core/Game/Game.h"
AISystem* AISystem::Instance = nullptr;

AISystem::AISystem()
{
	CurrentMode = EAINavigationMode::AStar;
	mesh = new NavigationMesh();

	Director = Engine::GetGame()->CreateAIDirector();
	//mesh->GenTestMesh();
}

AISystem::~AISystem()
{
	SafeDelete(mesh);
	SafeDelete(Director);
}

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

EAINavigationMode::Type AISystem::GetPathMode()
{
	if (Instance == nullptr) 
	{
		return EAINavigationMode::Limit;
	}
	return Instance->CurrentMode;
}
