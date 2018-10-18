#include "Mesh.h"
#include "Mesh.h"
#include "RHI/RHI.h"
#include "Core/Assets/MeshLoader.h"
#include "RHI/DeviceContext.h"
#include "Core/Utils/MemoryUtils.h"
#include "Material.h"
#include "Core/Assets/Archive.h"
#include "Core/Assets/SerialHelpers.h"

Mesh::Mesh()
{}

Mesh::Mesh(std::string filename, MeshLoader::FMeshLoadingSettings& Settings)
{
	LoadMeshFromFile(filename, Settings);
	FrameCreated = RHI::GetFrameCount();
	if (FrameCreated == 0)
	{
		FrameCreated = -10;
	}
}

void Mesh::Release()
{
	IRHIResourse::Release();
	MemoryUtils::DeleteReleaseableVector(SubMeshes);
	MemoryUtils::DeleteVector(Materials);
}

Mesh::~Mesh()
{}
void Mesh::Render(RHICommandList * list, bool SetMaterial)

{
	if (RHI::GetFrameCount() > FrameCreated + 1)
	{
		for (int i = 0; i < SubMeshes.size(); i++)
		{
			if (SubMeshes[i]->LoadSucessful)
			{
				if (SetMaterial)
				{
					TryPushMaterial(list, SubMeshes[i]->MaterialIndex);
				}
				list->SetVertexBuffer(SubMeshes[i]->VertexBuffers[list->GetDeviceIndex()]);
				list->SetIndexBuffer(SubMeshes[i]->IndexBuffers[list->GetDeviceIndex()]);
				list->DrawIndexedPrimitive((int)SubMeshes[i]->IndexBuffers[list->GetDeviceIndex()]->GetVertexCount(), 1, 0, 0, 0);
			}
		}
	}
}

void Mesh::TryPushMaterial(RHICommandList* list, int index)
{
	if (Materials.size() > index)
	{
		Materials[index]->SetMaterialActive(list);
	}
	else if (Materials.size() > 0)
	{
		Materials[Materials.size() - 1]->SetMaterialActive(list);
	}
}

void Mesh::LoadMeshFromFile(std::string filename, MeshLoader::FMeshLoadingSettings& Settings)
{
	MeshLoader::LoadMeshFromFile(filename, Settings, SubMeshes);
	if (SubMeshes.size() == 0)
	{
		Log::LogMessage("Failed to load mesh " + filename, Log::Severity::Error);
	}
}

void Mesh::SetMaterial(Material * mat, int index)
{
	if (Materials.size() > index)
	{
		Materials[index] = mat;
	}
	else
	{
		Materials.push_back(mat);
	}
}

Material * Mesh::GetMaterial(int index)
{
	if (Materials.size() > index)
	{
		return Materials[index];
	}
	return nullptr;
}

void SerialMaterial(Archive * A, Material* object)
{
	object->ProcessSerialArchive(A);
}

void Mesh::ProcessSerialArchive(Archive * A)
{
	A->LinkPropertyArray<Material>(Materials, "Mats", SerialMaterial);
}

void Mesh::SetShadow(bool state)
{
	DoesShadow = state;
}

bool Mesh::GetDoesShadow()
{
	return DoesShadow;
}

MeshEntity::MeshEntity(MeshLoader::FMeshLoadingSettings& Settings, std::vector<OGLVertex>& vertices, std::vector<int>& indices)
{
	if (vertices.size() == 0 || indices.size() == 0)
	{
		LoadSucessful = false;
		return;
	}
	const int count = Settings.InitOnAllDevices ? RHI::GetDeviceCount() : 1;
	for (int i = 0; i < count; i++)
	{
		VertexBuffers[i] = RHI::CreateRHIBuffer(ERHIBufferType::Vertex, RHI::GetDeviceContext(i));
		IndexBuffers[i] = RHI::CreateRHIBuffer(ERHIBufferType::Index, RHI::GetDeviceContext(i));
		VertexBuffers[i]->CreateVertexBuffer(sizeof(OGLVertex), sizeof(OGLVertex)* (int)vertices.size(), EBufferAccessType::Static);
		VertexBuffers[i]->UpdateVertexBuffer(vertices.data(), vertices.size());
		IndexBuffers[i]->CreateIndexBuffer(sizeof(int), sizeof(int)* (int)indices.size());
		IndexBuffers[i]->UpdateIndexBuffer(indices.data(), indices.size());
	}
	indices.clear();
	vertices.clear();
	LoadSucessful = true;
}

void MeshEntity::Release()
{
	MemoryUtils::DeleteReleaseableCArray(VertexBuffers, MAX_GPU_DEVICE_COUNT);
	MemoryUtils::DeleteReleaseableCArray(IndexBuffers, MAX_GPU_DEVICE_COUNT);
}
