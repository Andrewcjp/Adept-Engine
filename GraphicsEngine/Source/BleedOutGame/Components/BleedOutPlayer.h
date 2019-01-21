#pragma once
#include "Core/Components/Core_Components_FWD.h"
#include "Core/Components/Component.h"

class Health;
class WeaponManager;
struct WalkAudio
{
	float CurrnetTime = 0.0f;
	void Tick(GameObject* object, bool IsGrounded, glm::vec3 speed, bool isAI);
};
UCLASS()
class BleedOutPlayer :public Component
{
public:
	CLASS_BODY();
	BleedOutPlayer();
	~BleedOutPlayer();

	// Inherited via Component
	virtual void InitComponent() override;
	void OnCollide(CollisonData data) override;

	std::string GetInfoString();

	virtual void BeginPlay() override;
	void TickBleedout();
	virtual void Update(float delta) override;
	RigidbodyComponent* GetRBody()
	{
		return RB;
	}
	GameObject* CameraObject = nullptr;
	bool GetIsGrounded() const
	{
		return IsGrounded;
	}
	PROPERTY();
	glm::vec3 ExtraVel = glm::vec3();
	WeaponManager* Manager = nullptr;
	glm::vec3 GetColour();
	float BleedOutRate = 1.0f;
	float GetPlayerHealth();
private:
	void CheckForGround();
	void UpdateMovement(float delta);
	void TickAudio();
	glm::vec3 RelativeSpeed = glm::vec3();
	const float MaxSpeed = 10.0f;
	const float Acceleration = 35.0f;
	float MaxWalkableAngle = 40.0f;
	float jumpHeight = 10;
	RigidbodyComponent* RB = nullptr;
	CameraComponent* Cam = nullptr;
	float LookSensitivty = 0.1f;
	bool IsGrounded = false;
	bool LastFrameGrounded = false;
	const float AirSpeedFactor = 0.2f;
	bool Frontblocked = false;
	Health* Mhealth = nullptr;
	glm::vec3 CurrnetRot = glm::vec3(0);
	WalkAudio Walk;
	
	float SecondsLeft = 0.0f;
};


