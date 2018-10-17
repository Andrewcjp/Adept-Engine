#pragma once
#include <iostream>
#include <vector>
#include "RenderBaseTypes.h"
#include "Mesh.h"
#include "RHI/RHICommandList.h"
#include "RHI/RHI.h"
#include "Core/Assets/MeshLoader.h"
class Material;
struct MeshEntity
{
	RHIBuffer * VertexBuffers[MAX_GPU_DEVICE_COUNT] = { nullptr };
	RHIBuffer* IndexBuffers[MAX_GPU_DEVICE_COUNT] = { nullptr };
	MeshEntity(MeshLoader::FMeshLoadingSettings& Settings, std::vector<OGLVertex> &vertices, std::vector<int> &indices);
	void Release();
	bool LoadSucessful = false;
	int MaterialIndex = 0;
};
class Mesh :public IRHIResourse
{
public:
	RHI_API Mesh();
	Mesh(std::string filename, MeshLoader::FMeshLoadingSettings& Settings);

	RHI_API ~Mesh();
	void Render(RHICommandList * list, bool SetMaterial = false);
	void TryPushMaterial(RHICommandList * list, int index);
	void LoadMeshFromFile(std::string filename, MeshLoader::FMeshLoadingSettings & Settings);
	std::string AssetName = "";
	void SetMaterial(Material* mat, int index);
	Material* GetMaterial(int index);
	void ProcessSerialArchive(class Archive* Arch);
	void SetShadow(bool state);
	bool GetDoesShadow();
private:
	RHI_API void Release() override;
	int FrameCreated = 0;
	std::vector<Material*> Materials;
	/**
	*\brief MeshEntity holds all the information to render a mesh section there might be multiple per mesh object
	*/
	std::vector<MeshEntity*> SubMeshes;
	int MaterialCount = 0;
	bool DoesShadow = true;
};

