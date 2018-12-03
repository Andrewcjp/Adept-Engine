#include "AISystem.h"
#include "AI/Generation/NavMeshGenerator.h"
#include "AIDirector.h"
#include "Behaviour/BehaviourTreeManager.h"
#include "Core/Engine.h"
#include "Core/Game/Game.h"
#include "Navigation/NavigationMesh.h"
#include "Core/Platform/ConsoleVariable.h"
static ConsoleVariable DebugModeVar("aidebug", 0, ECVarType::ConsoleAndLaunch);
AISystem* AISystem::Instance = nullptr;
AISystem::AISystem()
{
	CurrentMode = EAINavigationMode::DStarLTE;
	mesh = new NavigationMesh();
	BTManager = new BehaviourTreeManager();

	DebugMode = EAIDebugMode::PathOnly;
}

AISystem::~AISystem()
{
	SafeDelete(mesh);
	SafeDelete(Director);
}

void AISystem::SetupForScene(Scene* newscene)
{
	SceneStart();
	Director->SetScene(newscene);	
}

void AISystem::SceneStart()
{
	Director = Engine::GetGame()->CreateAIDirector();
}

void AISystem::SceneEnd()
{
	SafeDelete(Director);
	BTManager->Reset();
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
	SetDebugMode(DebugModeVar.GetAsEnum<EAIDebugMode::Type>());
	Director->Tick();
	BTManager->Tick(dt);
	if (AISystem::GetDebugMode() == EAIDebugMode::NavMesh || AISystem::GetDebugMode() == EAIDebugMode::All)
	{
		mesh->RenderMesh();
	}
}
void AISystem::EditorTick()
{
	SetDebugMode(DebugModeVar.GetAsEnum<EAIDebugMode::Type>());
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
		Log::LogMessage("CalculatePath Failed"+ NavigationMesh::GetErrorCodeAsString(result), Log::Severity::Warning);
	}
	return result;
}

void AISystem::GenerateMesh()
{
#if TDSIM_ENABLED
	return;
#endif
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
