#include "SpawnMarker.h"
#include "AISystem.h"
#include "AIDirector.h"
#include "Core/Components/ComponentRegistry.h"


SpawnMarker::SpawnMarker()
{
	TypeID = ComponentRegistry::BaseComponentTypes::SpawnMarkerComp;
}


SpawnMarker::~SpawnMarker()
{}

void SpawnMarker::BeginPlay()
{
	AISystem::GetDirector<AIDirector>()->spawnmarkers.push_back(this);
}

