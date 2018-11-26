#pragma once
#include "Core/Components/Component.h"
#include "AI/Core/Navigation/NavigationMesh.h"
class GameObject;
class RigidbodyComponent;
class AIController : public Component
{
public:
	CORE_API AIController();
	~AIController();
	CORE_API void MoveTo(glm::vec3 pos);
	CORE_API void MoveTo(GameObject* target);
	bool LookAtTarget = true;
private:
	struct Target 
	{
		glm::vec3 StaticPos = glm::vec3(0,0,0);
		GameObject* MovingTarget = nullptr;
		glm::vec3 GetTargetPos();
		bool IsValid = false;
	};
	Target CurrentTarget = Target();
	// Inherited via Component
	virtual void InitComponent() override;
	void Update(float dt) override;
	void ReplanPath();
	const float ThresholdToRequestReplan = 1.0f;
	NavigationPath* Path;
	RigidbodyComponent* Rigidbody = nullptr;
	float Speed = 15.0f;
	int CurrentPathIndex = 0;
	const float PathNodeArriveRaduis = 2.0f;
};

