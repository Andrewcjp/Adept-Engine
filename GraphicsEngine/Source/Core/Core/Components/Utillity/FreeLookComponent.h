#pragma once
#include "Core/Components/Component.h"

class FreeLookComponent : public Component
{
public:
	FreeLookComponent();
	~FreeLookComponent();

	// Inherited via Component
	virtual void InitComponent() override;
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;

private:
	const float BaseTranslateSpeed = 10.0f;
	const float FastTranslateSpeed = 100.0f;
	float TranslateSpeed = 10.0f;
	float LookSensitivty = 0.001f;
	class CameraComponent* CameraComp = nullptr;
	class Camera* Cam = nullptr;
};

