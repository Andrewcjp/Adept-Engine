#pragma once
#include "Core/Components/Component.h"
#include "AI/Core/Navigation/NavigationManager.h"
#include "Core/Types/WeakObjectPtr.h"
class GameObject;
class RigidbodyComponent;
class AIController : public Component
{
public:
	CORE_API AIController();
	~AIController();
	CORE_API void MoveTo(glm::vec3 pos);
	CORE_API void MoveTo(GameObject* target);
	bool LookAtTarget = false;
	float TurnRatio = 0.5f;
	float Speed = 10.0f;
	CORE_API void SetLookAt(glm::vec3 pos);
	bool Active = true;
private:
	struct Target 
	{
		glm::vec3 StaticPos = glm::vec3(0,0,0);
		WeakObjectPtr<GameObject> MovingTarget;
		glm::vec3 GetTargetPos();
		bool IsValid = false;
	};
	Target CurrentTarget = Target();
	// Inherited via Component
	virtual void InitComponent() override;
	void Update(float dt) override;
	void ReplanPath();
	const float ThresholdToRequestReplan = 1.0f;
	NavigationPath Path;
	RigidbodyComponent* Rigidbody = nullptr;

	int CurrentPathIndex = 0;
	const float PathNodeArriveRaduis = 4.0f;
	float LastAngle = 0.0f;
};

