#pragma once
#include "Core\Components\Component.h"

class CameraComponent;
class SpectatorComponent : public Component
{
public:
	SpectatorComponent();
	~SpectatorComponent();

	virtual void BeginPlay() override;
	virtual void Update(float delta) override;
private:
	CameraComponent* Comp = nullptr;

};

