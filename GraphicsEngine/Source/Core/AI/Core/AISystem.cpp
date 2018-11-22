#include "AISystem.h"
#include "AI/Generation/NavMeshGenerator.h"
#include "AIDirector.h"
#include "Behaviour/BehaviourTreeManager.h"
#include "Core/Engine.h"
#include "Core/Game/Game.h"
#include "Navigation/NavigationMesh.h"

AISystem* AISystem::Instance = nullptr;
AISystem::AISystem()
{
	CurrentMode = EAINavigationMode::DStarLTE;
	mesh = new NavigationMesh();
	BTManager = new BehaviourTreeManager();
	Director = Engine::GetGame()->CreateAIDirector();
	DebugMode = EAIDebugMode::PathOnly;
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
	if (AISystem::GetDebugMode() == EAIDebugMode::NavMesh || AISystem::GetDebugMode() == EAIDebugMode::All)
	{
		mesh->RenderMesh();
	}
}
void AISystem::EditorTick()
{
	if (AISystem::GetDebugMode() == EAIDebugMode::NavMesh || AISystem::GetDebugMode() == EAIDebugMode::All)
	{
		mesh->RenderMesh();
	}
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
	ENavRequestStatus::Type result = mesh->CalculatePath(Startpoint, EndPos, outpath);
	if (result != ENavRequestStatus::Complete)
	{
		Log::LogMessage("CalculatePath Failed", Log::Severity::Warning);
	}
	return result;
}

void AISystem::GenerateMesh()
{
	return;
	n = new NavMeshGenerator();
	n->Voxelise(nullptr);
	mesh->Plane = n->GetPlane(-17);
}

EAIDebugMode::Type AISystem::GetDebugMode()
{
	return Get()->DebugMode;
}

void AISystem::SetDebugMode(EAIDebugMode::Type mode)
{
	Get()->DebugMode = mode;
}
