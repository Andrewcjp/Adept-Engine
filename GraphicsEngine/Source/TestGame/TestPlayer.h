#pragma once
#include "Core/Components/Component.h"
#include "Core/Components/Core_Components_FWD.h"
class TestPlayer :public Component
{
public:
	TestPlayer();
	~TestPlayer();

	// Inherited via Component
	virtual void InitComponent() override;
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;
	GameObject* CameraObject = nullptr;
private:
	
	float Speed = 10;
	float jumpHeight = 10;
	RigidbodyComponent* RB = nullptr;
	CameraComponent* Cam = nullptr;
	float LookSensitivty = 0.1f;
};

