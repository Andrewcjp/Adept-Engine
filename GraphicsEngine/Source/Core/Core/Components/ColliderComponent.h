#pragma once
#include "Component.h"
#include "physx\PxShape.h"
class ColliderComponent :
	public Component
{
public:
	enum ShapeType
	{
		Box,
		Sphere,
		Mesh
	};
	ColliderComponent();
	~ColliderComponent();

	// Inherited via Component
	virtual void InitComponent() override;
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;	
	
	ShapeType GetCollisonShape();
	void SetCollisonShape(ShapeType newtype);
	class physx::PxShape* Internal_GetPhysxShape();
private:
	void CreateShape();
	ShapeType CollisionShapeType;
	ShapeType InitalisedCollisionShapeType;
	physx::PxShape* CollisionShape;
	float Radius = 1;
	float BoxHalfExtent = 1;
};

