#include "SpawnMarker.h"
#include "AISystem.h"
#include "AIDirector.h"
#include "Core/Components/CompoenentRegistry.h"


SpawnMarker::SpawnMarker()
{
	TypeID = CompoenentRegistry::BaseComponentTypes::SpawnMarkerComp;
}


SpawnMarker::~SpawnMarker()
{}

void SpawnMarker::BeginPlay()
{
	AISystem::GetDirector<AIDirector>()->spawnmarkers.push_back(this);
}

