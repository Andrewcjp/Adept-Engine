#pragma once
#include "Component.h"
#include "Rendering/Core/Mesh.h"
#include "Rendering/Core/Material.h"
#include "Core/Reflection/IReflect.h"
#include "Core/Assets/AssetPtr.h"
UCLASS()
class LowLevelAccelerationStructure;
class MeshAsset;
class MaterialAsset;
class MeshRendererComponent :
	public Component
{
public:
	CLASS_BODY_Reflect();
	MeshRendererComponent();
	CORE_API MeshRendererComponent(Mesh * Mesh, Material * materal);
	virtual ~MeshRendererComponent();
	void SetUpMesh(Mesh * Mesh, Material * materal);
	Material* GetMaterial(int index);
	CORE_API Mesh* GetMesh();
	CORE_API void SetMaterial(Material * mat, int index);
	CORE_API void SetVisiblity(bool state);
	CORE_API void LoadAnimation(std::string filename, std::string name);
	CORE_API void LoadAnimation(std::string filename, std::string name, MeshLoader::FMeshLoadingSettings & Settings);
	CORE_API void PlayAnim(std::string name);
	CORE_API glm::vec3 GetPosOfBone(std::string Name);
	void PrepareDataForRender();
	CORE_API virtual void SceneInitComponent() override;
	CORE_API virtual void OnTransformUpdate() override;
	LowLevelAccelerationStructure* GetAccelerationStructure()const;
	void SetMaterialAsset(std::string path);

	void Serialize(BinaryArchive* Achive) override;

	void OnPropertyUpdate(ClassReflectionNode* Node) override;

private:
	PROPERTY();
	AssetPtr<MeshAsset> m_pMeshAsset;
	PROPERTY();
	AssetPtr<MaterialAsset> m_pMaterialAsset;
	PROPERTY();
	std::string AssetPath = "";
	PROPERTY();
	std::string MaterialPath = "";
	std::vector<LowLevelAccelerationStructure*> MeshAcclerations = { nullptr };
	Mesh* m_mesh = nullptr;
	// Inherited via Component
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;

	bool IsVisible = true;
	// Inherited via Component
	virtual void InitComponent() override;
	virtual void ProcessSerialArchive(class Archive* Arch) override;
};

