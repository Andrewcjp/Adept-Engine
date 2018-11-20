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
	void LoadMesh();
	void SceneInitComponent()override;
	void TransferToRigidbody();
	void SetTriangleMeshAssetName(std::string name);

#if WITH_EDITOR
	virtual void EditorUpdate();
	void GetInspectorProps(std::vector<InspectorProperyGroup>& props);
#endif
	//todo: refactor
	float Radius = 1.0f;
	glm::vec3 BoxExtents = glm::vec3(1, 1, 1);
	float Height = 1.0f;
private:
	EShapeType::Type CollisionShapeType = EShapeType::eBOX;

	RigidBody* Actor = nullptr;
	std::string MeshName;
	std::vector<glm::vec3*> Points;

};

