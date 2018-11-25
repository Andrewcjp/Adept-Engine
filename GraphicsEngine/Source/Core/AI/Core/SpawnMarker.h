#pragma once
#include "Core/Components/Component.h"
class SpawnMarker : public Component
{
public:
	SpawnMarker();
	~SpawnMarker();

	virtual void BeginPlay() override;
};

