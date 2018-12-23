#pragma once
#include "Core/Components/Core_Components_FWD.h"
#include "Core/Components/Component.h"

class Health;
class WeaponManager;
class TestPlayer :public Component
{
public:
	TestPlayer();
	~TestPlayer();

	// Inherited via Component
	virtual void InitComponent() override;
	void OnCollide(CollisonData data) override;

	std::string GetInfoString();
	
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;
	RigidbodyComponent* GetRBody() { return RB; }
	GameObject* CameraObject = nullptr;
	bool GetIsGrounded() const { return IsGrounded; }
	glm::vec3 ExtraVel = glm::vec3();
	WeaponManager* Manager = nullptr;
private:
	void CheckForGround();
	void UpdateMovement(float delta);
	glm::vec3 RelativeSpeed = glm::vec3();
	const float MaxSpeed = 10.0f;
	const float Acceleration = 35.0f;
	float MaxWalkableAngle = 40.0f;
	float jumpHeight = 10;
	RigidbodyComponent* RB = nullptr;
	CameraComponent* Cam = nullptr;
	float LookSensitivty = 0.1f;
	bool IsGrounded = false;
	const float AirSpeedFactor = 0.2f;
	bool Frontblocked = false;
	Health* Mhealth = nullptr;
	glm::vec3 CurrnetRot = glm::vec3(0);
};

