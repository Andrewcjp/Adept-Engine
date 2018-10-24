#pragma once
#include "Core/Components/Component.h"
class SpawningPool : public Component
{
public:
	SpawningPool();
	~SpawningPool();

	// Inherited via Component
	virtual void InitComponent() override;
	void OnDestroy() override;
};

