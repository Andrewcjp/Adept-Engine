#pragma once
#include "Component.h"
#include "Physics/PhysicsTypes.h"
class ColliderComponent :
	public Component
{
public:
	
	ColliderComponent();
	~ColliderComponent();

	// Inherited via Component
	virtual void InitComponent() override;
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;	
	void ProcessSerialArchive(class Archive* A) override;
	EShapeType::Type GetCollisonShape();
	void SetCollisonShape(EShapeType::Type newtype);
	ShapeElem* GetColliderShape();
	void GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props);

private:
	EShapeType::Type CollisionShapeType = EShapeType::eBOX;
	float Radius = 1;
	glm::vec3 BoxExtents = glm::vec3(1, 1, 1);
};

