#include "Mesh.h"
#include "Mesh.h"
#include "Material.h"
#include "Core/Assets/Archive.h"
#include "Core/Assets/SerialHelpers.h"
#include "Rendering/Shaders/Shader_SkeletalMesh.h"
#include "Core/Engine.h"
#include "Core/Assets/ImageIO.h"

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
	SafeRelease(pSkeletalEntity);	
}

Mesh::~Mesh()
{}

void Mesh::Render(RHICommandList * list, bool SetMaterial)
{
	if (RHI::GetFrameCount() > FrameCreated + 1)
	{
		if (pSkeletalEntity != nullptr && SetMaterial)
		{
			pSkeletalEntity->Tick(Engine::GetDeltaTime());
			//todo: integrate into material system
			list->SetPipelineStateObject(ShaderComplier::GetShader<Shader_SkeletalMesh>());			
			list->SetTexture(Materials[0]->GetTexturebind(Material::DefuseBindName), 9);
			ShaderComplier::GetShader<Shader_SkeletalMesh>()->PushBones(pSkeletalEntity->FinalBoneTransforms, list);
			for (int i = 0; i < pSkeletalEntity->MeshEntities.size(); i++)
			{				
				list->SetVertexBuffer(pSkeletalEntity->MeshEntities[i]->VertexBuffers[list->GetDeviceIndex()]);
				list->SetIndexBuffer(pSkeletalEntity->MeshEntities[i]->IndexBuffers[list->GetDeviceIndex()]);
				list->DrawIndexedPrimitive((int)pSkeletalEntity->MeshEntities[i]->IndexBuffers[list->GetDeviceIndex()]->GetVertexCount(), 1, 0, 0, 0);
			}
		}
		else
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
	ImportSettings = Settings;
	MeshLoader::LoadMeshFromFile(filename, Settings, SubMeshes, &pSkeletalEntity);
	if (SubMeshes.size() == 0 && pSkeletalEntity == nullptr)
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

SkeletalMeshEntry * Mesh::GetSkeletalMesh() const
{
	return pSkeletalEntity;
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
