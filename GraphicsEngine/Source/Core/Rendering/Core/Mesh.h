#pragma once
#include <iostream>
#include <vector>
#include "RenderBaseTypes.h"
#include "Mesh.h"
#include "RHI/RHICommandList.h"
#include "RHI/RHI.h"
#include "Core/Assets/MeshLoader.h"
class Mesh :public IRHIResourse
{
public:
	RHI_API Mesh();
	Mesh(std::string filename, MeshLoader::FMeshLoadingSettings& Settings);

	RHI_API ~Mesh();
	virtual void Render(RHICommandList* list);
	void LoadMeshFromFile(std::string filename, MeshLoader::FMeshLoadingSettings & Settings);
	std::string AssetName = "";
private:
	RHI_API void Release() override;
	
	RHIBuffer * VertexBuffers[MAX_DEVICE_COUNT] = { nullptr };
	RHIBuffer* IndexBuffers[MAX_DEVICE_COUNT] = { nullptr };
};

