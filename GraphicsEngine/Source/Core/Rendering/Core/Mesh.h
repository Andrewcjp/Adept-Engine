#pragma once
#include <iostream>
#include "RenderBaseTypes.h"
#include "Mesh.h"
#include "RHI/RHI.h"
#include "Core/Assets/MeshLoader.h"
#include "Core/IRefCount.h"
#include "Culling/CullingAABB.h"

#include "RHI/DeviceContext.h"
#include "RHI/RHITemplates.h"
#include "RHI/BaseTexture.h"
#include "Core/ObjectBase/SharedPtr.h"
#include "RHI/RHICommandList.h"
class Material;
struct SkeletalMeshEntry;
class MeshBatch;
class CullingAABB;
class MeshRendererComponent;
class RHIBuffer;
class RHIBufferGroup;
struct MeshEntity
{
	RHIBufferGroup* VertexBuffers =  nullptr ;
	RHIBufferGroup* IndexBuffers =  nullptr ;
	MeshEntity(MeshLoader::FMeshLoadingSettings& Settings, std::vector<OGLVertex> &vertices, std::vector<IndType> &indices);
	MeshEntity();
	void InstanceElement(MeshEntity * other, MeshLoader::FMeshLoadingSettings& Settings);
	bool LoadSucessful = false;
	int MaterialIndex = 0;
	CullingAABB* AABB = nullptr;
};
class Mesh : public IRHIResourse
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
	CullingAABB* GetBounds();
	void UpdateBounds(glm::vec3 pos, glm::vec3 scale);
	MeshRendererComponent* Renderer = nullptr;
	/**
	*\brief MeshEntity holds all the information to render a mesh section there might be multiple per mesh object
	*/
	bool ReCreate = false;
	std::vector<MeshEntity*> SubMeshes;
	void InvalidateBatch() { ReCreate = true; }
private:
	CullingAABB MeshBounds;
	int FrameCreated = 0;
	std::vector<Material*> Materials;

	SkeletalMeshEntry* pSkeletalEntity = nullptr;
	int MaterialCount = 0;
	bool DoesShadow = true;
	RHIBufferGroup* PrimitiveTransfromBuffer = nullptr;
};

