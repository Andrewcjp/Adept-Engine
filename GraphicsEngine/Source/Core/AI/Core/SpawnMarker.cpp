#include "SpawnMarker.h"
#include "AISystem.h"
#include "AIDirector.h"



SpawnMarker::SpawnMarker()
{
	
}


SpawnMarker::~SpawnMarker()
{}

void SpawnMarker::BeginPlay()
{
	AISystem::GetDirector<AIDirector>()->spawnmarkers.push_back(this);
}

