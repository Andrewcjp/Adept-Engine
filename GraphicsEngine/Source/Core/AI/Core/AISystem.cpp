#include "Stdafx.h"
#include "AISystem.h"
#include "Navigation/NavigationMesh.h"
#include "AIDirector.h"
#include "core/Engine.h"
#include "core/Game/Game.h"
#include "Behaviour/BehaviourTreeManager.h"
AISystem* AISystem::Instance = nullptr;

AISystem::AISystem()
{
	CurrentMode = EAINavigationMode::AStar;
	mesh = new NavigationMesh();
	BTManager = new BehaviourTreeManager();
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

void AISystem::Tick(float dt)
{
	Director->Tick();
	BTManager->Tick(dt);
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
ENavRequestStatus::Type AISystem::CalculatePath(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath** outpath)
{
	return mesh->CalculatePath(Startpoint, EndPos, outpath);
}