#pragma once
#include <iostream>
#include "RenderBaseTypes.h"
#include "Mesh.h"
#include "RHI/RHICommandList.h"
#include "RHI/RHI.h"
#include "Core/Assets/MeshLoader.h"
#include "Core/IRefCount.h"
class Material;
struct SkeletalMeshEntry;
class MeshBatch;
struct MeshEntity
{
	SharedPtr<RHIBuffer> VertexBuffers[MAX_GPU_DEVICE_COUNT] = { nullptr };
	SharedPtr<RHIBuffer> IndexBuffers[MAX_GPU_DEVICE_COUNT] = { nullptr };
	MeshEntity(MeshLoader::FMeshLoadingSettings& Settings, std::vector<OGLVertex> &vertices, std::vector<IndType> &indices);
	MeshEntity();
	void InstanceElement(MeshEntity * other, MeshLoader::FMeshLoadingSettings& Settings);
	bool LoadSucessful = false;
	int MaterialIndex = 0;
	int BaseVertex = 0;
};
class Mesh :  public IRHIResourse
{
public:
	RHI_API Mesh();
	Mesh(std::string filename, MeshLoader::FMeshLoadingSettings& Settings);
	void InstanceFrom(Mesh* m);
	RHI_API ~Mesh();
	void Tick(float dt);
	void Render(RHICommandList * list, bool SetMaterial = false);
	void TryPushMaterial(RHICommandList * list, int index);
	void LoadMeshFromFile(std::string filename, MeshLoader::FMeshLoadingSettings & Settings);
	std::string AssetName = "";
	void SetMaterial(Material* mat, int index);
	Material* GetMaterial(int index);
	void ProcessSerialArchive(class Archive* Arch);
	void SetShadow(bool state);
	bool GetDoesShadow();
	MeshLoader::FMeshLoadingSettings ImportSettings;
	SkeletalMeshEntry* GetSkeletalMesh()const;
	glm::vec3 GetPosOfBone(std::string Name);
	MeshBatch* GetMeshBatch();
	void PrepareDataForRender(GameObject* Parent);
	bool IsVisible = true;
	RHI_API void Release() override;
private:

	int FrameCreated = 0;
	std::vector<Material*> Materials;
	/**
	*\brief MeshEntity holds all the information to render a mesh section there might be multiple per mesh object
	*/
	std::vector<MeshEntity*> SubMeshes;
	SkeletalMeshEntry* pSkeletalEntity = nullptr;
	int MaterialCount = 0;
	bool DoesShadow = true;
	RHIBuffer* PrimitiveTransfromBuffer;
};

