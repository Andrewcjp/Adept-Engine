#pragma once
#include <iostream>
#include <vector>
#include "RenderBaseTypes.h"
#include "Renderable.h"
#include "RHI/RHICommandList.h"
#include "RHI/RHI.h"
#include "Core/Assets/MeshLoader.h"
class Mesh :public Renderable
{
public:
	Mesh();
	Mesh(std::string filename, MeshLoader::FMeshLoadingSettings& Settings);
	~Mesh();
	void Render(RHICommandList* list) override;
	void LoadMeshFromFile(std::string filename, MeshLoader::FMeshLoadingSettings & Settings);
private:
	RHIBuffer * VertexBuffers[MAX_DEVICE_COUNT] = { nullptr };
	RHIBuffer* IndexBuffers[MAX_DEVICE_COUNT] = { nullptr };
};

