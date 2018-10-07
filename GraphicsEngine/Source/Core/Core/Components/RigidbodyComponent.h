#pragma once
#include "Component.h"
#include "Physics/Physics_fwd.h"
#include "Physics/PhysicsEngine.h"
class RigidbodyComponent :
	public Component
{
public:
	CORE_API RigidbodyComponent();
	CORE_API ~RigidbodyComponent();

	// Inherited via Component
	void BeginPlay() override;
	void Update(float delta) override;
	void FixedUpdate(float delta) override;
	void MovePhysicsBody(glm::vec3 newpos, glm::quat newrot);
	virtual void SceneInitComponent() override;
	CORE_API void SetLinearVelocity(glm::vec3 velocity);
	CORE_API glm::vec3 GetVelocity();
	CORE_API void SetLockFlags(BodyInstanceData data);
	CORE_API BodyInstanceData GetLockFlags();
	void OnTransformUpdate() override ;
private:
	RigidBody* actor;
	BodyInstanceData LockData;
	// Inherited via Component
#if WITH_EDITOR
	void GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props) override;
#endif
	void ProcessSerialArchive(Archive * A) override;

	float mass = 1.0f;
	// Inherited via Component
	virtual void InitComponent()override {} ;
	glm::vec3 InitalVelocity = glm::vec3(0);

};

