#pragma once
#include "Component.h"
#include "Physics/PhysicsTypes.h"

class RigidbodyComponent;
class ColliderComponent :
	public Component
{
public:

	CORE_API ColliderComponent();
	CORE_API ~ColliderComponent();
	CORE_API void SetEnabled(bool State);
	// Inherited via Component
	virtual void InitComponent() override;
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;
	void ProcessSerialArchive(class Archive* A) override;
	CORE_API EShapeType::Type GetCollisonShape();
	CORE_API void SetCollisonShape(EShapeType::Type newtype);

	Collider* GetCollider();
	void LoadMesh();
	void SceneInitComponent()override;
	void MovePhysicsBody(glm::vec3 newpos, glm::quat newrot);
	void TransferToRigidbody();
	void SetTriangleMeshAssetName(std::string name);
	void RenderShape();
#if WITH_EDITOR
	virtual void EditorUpdate();
	void GetInspectorProps(std::vector<InspectorProperyGroup>& props);
#endif
	//todo: refactor
	float Radius = 1.0f;
	glm::vec3 BoxExtents = glm::vec3(1, 1, 1);
	glm::vec3 LocalOffset = glm::vec3(0, 0, 0);
	float Height = 1.0f;
	bool IsTrigger = false;
	bool IsEnabled()const
	{
		return Enabled;
	}
	RigidBody* GetActor()
	{
		return Actor;
	}
private:
	CORE_API ShapeElem* GetColliderShape();
	EShapeType::Type CollisionShapeType = EShapeType::eBOX;
	RigidbodyComponent* RigidComp = nullptr;
	Collider*	CurrentCollider = nullptr;
	RigidBody* Actor = nullptr;
	std::string MeshName;
	std::vector<glm::vec3*> Points;
	bool Enabled = true;
};

