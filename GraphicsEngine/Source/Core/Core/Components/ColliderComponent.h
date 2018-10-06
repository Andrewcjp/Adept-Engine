#pragma once
#include "Component.h"
#include "Physics/PhysicsTypes.h"
class ColliderComponent :
	public Component
{
public:
	
	CORE_API ColliderComponent();
	CORE_API ~ColliderComponent();

	// Inherited via Component
	virtual void InitComponent() override;
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;	
	void ProcessSerialArchive(class Archive* A) override;
	CORE_API EShapeType::Type GetCollisonShape();
	CORE_API void SetCollisonShape(EShapeType::Type newtype);
	CORE_API ShapeElem* GetColliderShape();
	void SceneInitComponent()override;
	void TransferToRigidbody();
#if WITH_EDITOR
	void GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props);
#endif
private:
	EShapeType::Type CollisionShapeType = EShapeType::eBOX;
	float Radius = 1;
	glm::vec3 BoxExtents = glm::vec3(1, 1, 1);
	RigidBody* Actor = nullptr;
};

