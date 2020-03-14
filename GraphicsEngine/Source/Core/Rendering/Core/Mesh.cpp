#include "Mesh.h"
#include "Mesh.h"
#include "Material.h"
#include "Core/Assets/Archive.h"
#include "Core/Assets/SerialHelpers.h"
#include "Rendering/Shaders/Shader_SkeletalMesh.h"
#include "Core/Engine.h"
#include "Core/Assets/ImageIO.h"
#include "Mesh/MeshBatch.h"
#include "SceneRenderer.h"
#include "Core/GameObject.h"
#include "Core/Transform.h"
#include "Culling/CullingAABB.h"
#include "Core/Components/Component.h"
#include "Core/Components/MeshRendererComponent.h"
#include "RHI/RHIBufferGroup.h"

Mesh::Mesh()
{
	FrameCreated = RHI::GetFrameCount();
	if (FrameCreated == 0)
	{
		FrameCreated = -10;
	}
	PrimitiveTransfromBuffer = new RHIBufferGroup();
	PrimitiveTransfromBuffer->CreateConstantBuffer(sizeof(MeshTransfromBuffer), 1, true);
}

Mesh::Mesh(std::string filename, MeshLoader::FMeshLoadingSettings& Settings) :Mesh()
{
	LoadMeshFromFile(filename, Settings);
}

void Mesh::InstanceFrom(Mesh * m)
{
	ImportSettings = m->ImportSettings;
	for (int i = 0; i < m->SubMeshes.size(); i++)
	{
		MeshEntity* ME = new MeshEntity();
		ME->InstanceElement(m->SubMeshes[i], m->ImportSettings);
		SubMeshes.push_back(ME);
	}
	for (int i = 0; i < m->Materials.size(); i++)
	{
		Materials.push_back(m->Materials[i]);
	}
	AssetName = m->AssetName;
}

void Mesh::Release()
{
	IRHIResourse::Release();
	MemoryUtils::DeleteVector(SubMeshes);
	//MemoryUtils::DeleteVector(Materials);
	SafeRHIRelease(PrimitiveTransfromBuffer);
	//	SafeRelease(pSkeletalEntity);
}

CullingAABB * Mesh::GetBounds()
{
	return &MeshBounds;
}

void Mesh::UpdateBounds(glm::vec3 pos, glm::vec3 scale)
{
	MeshBounds.Reset();
	for (int i = 0; i < SubMeshes.size(); i++)
	{
		MeshBounds.AddAABB(SubMeshes[i]->AABB);
	}
	glm::vec3 localPos = MeshBounds.GetPos() * scale;
	MeshBounds.SetPos(pos + localPos);
	MeshBounds.SetScale(scale);

}

Mesh::~Mesh()
{}

void Mesh::Tick(float dt)
{
	if (pSkeletalEntity != nullptr)
	{
		pSkeletalEntity->Tick(dt);
	}
}

void Mesh::Render(RHICommandList * list, bool SetMaterial)
{
#if 1
	if (RHI::GetFrameCount() > FrameCreated + 1)
	{
		//if (pSkeletalEntity != nullptr && SetMaterial)
		//{
		//	//todo: integrate into material system
		//	list->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_SkeletalMesh>()));
		//	list->SetTexture(Materials[0]->GetTexturebind(Material::DefuseBindName), 9);
		//	ShaderComplier::GetShader<Shader_SkeletalMesh>()->PushBones(pSkeletalEntity->FinalBoneTransforms, list);
		//	for (int i = 0; i < pSkeletalEntity->MeshEntities.size(); i++)
		//	{
		//		list->SetVertexBuffer(pSkeletalEntity->MeshEntities[i]->VertexBuffers[list->GetDeviceIndex()].Get());
		//		list->SetIndexBuffer(pSkeletalEntity->MeshEntities[i]->IndexBuffers[list->GetDeviceIndex()].Get());
		//		list->DrawIndexedPrimitive((int)pSkeletalEntity->MeshEntities[i]->IndexBuffers[list->GetDeviceIndex()]->GetVertexCount(), 1, 0, 0, 0);
		//	}
		//}
		//else
		{
			for (int i = 0; i < SubMeshes.size(); i++)
			{
				if (SubMeshes[i]->LoadSucessful)
				{
					if (SetMaterial)
					{
						TryPushMaterial(list, SubMeshes[i]->MaterialIndex);
					}
					list->SetVertexBuffer(SubMeshes[i]->VertexBuffers->Get(list));
					list->SetIndexBuffer(SubMeshes[i]->IndexBuffers->Get(list));
					list->DrawIndexedPrimitive((int)SubMeshes[i]->IndexBuffers->Get(list)->GetVertexCount(), 1, 0, 0, 0);
				}
			}
		}
	}
#endif
}
//#TODO Remove
void Mesh::TryPushMaterial(RHICommandList* list, int index)
{
	if (Materials.size() > index)
	{
		//Materials[index]->SetMaterialActive(list);
	}
	else if (Materials.size() > 0)
	{
		//Materials[Materials.size() - 1]->SetMaterialActive(list);
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
	if (Materials.size() > 0)
	{
		return Materials[Materials.size() - 1];
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

glm::vec3 Mesh::GetPosOfBone(std::string Name)
{
	if (GetSkeletalMesh() == nullptr)
	{
		return glm::vec3();
	}
	auto itor = GetSkeletalMesh()->m_BoneMapping.find(Name);
	if (itor == GetSkeletalMesh()->m_BoneMapping.end())
	{
		Log::LogMessage("No Bone Called " + Name, Log::Warning);
		return glm::vec3();
	}
	uint boneID = itor->second;
	glm::mat4x4 Boneitor = GetSkeletalMesh()->FinalBoneTransforms[boneID];
	//glm::vec3 LocalPois = glm::vec3(1, 1, 1);
	//LocalPois = Boneitor * glm::vec4(LocalPois, 0.0f);
	return  glm::vec3(Boneitor[3][0], Boneitor[3][1], Boneitor[3][2]);
}

MeshBatch * Mesh::GetMeshBatch()
{
	//if (RHI::GetFrameCount() <= FrameCreated + 1)
	//{
	//	return nullptr;
	//}
	//todo: handle multiGPU
	MeshBatch* B = new MeshBatch();

	for (int i = 0; i < SubMeshes.size(); i++)
	{
		if (!SubMeshes[i]->LoadSucessful)
		{
			continue;
		}
		MeshBatchElement* e = new MeshBatchElement();
		e->VertexBuffer = SubMeshes[i]->VertexBuffers;
		e->IndexBuffer = SubMeshes[i]->IndexBuffers;
		e->NumPrimitives = (int)SubMeshes[i]->IndexBuffers->Get(0)->GetVertexCount();
		e->NumInstances = 1;
		e->TransformBuffer = PrimitiveTransfromBuffer;
		e->MaterialInUse = GetMaterial(SubMeshes[i]->MaterialIndex);
		e->IsVisible = IsVisible;
		if (e->MaterialInUse != nullptr)
		{
			e->bTransparent = e->MaterialInUse->GetRenderPassType() == EMaterialRenderType::Transparent;
		}
		B->AddMeshElement(e);
	}
	if (Renderer != nullptr)
	{
		B->MainPassCulled = Renderer->GetOwner()->IsCulled(ECullingPass::MainPass);
		B->ShadowPassCulled = Renderer->GetOwner()->IsCulled(ECullingPass::ShadowPass);
		B->CastShadow = GetDoesShadow();
		B->Owner = Renderer->GetOwner();
	}
	return B;
}

void Mesh::PrepareDataForRender(GameObject* parent)
{
	/*if (parent->GetLastMovedFrame() < RHI::GetFrameCount())
	{
		return;
	}*/
	MeshTransfromBuffer SCB = {};
	SCB.M = parent->GetTransform()->GetModel();
	PrimitiveTransfromBuffer->UpdateConstantBuffer(&SCB, 0);
}

MeshEntity::MeshEntity(MeshLoader::FMeshLoadingSettings& Settings, std::vector<OGLVertex>& vertices, std::vector<IndType>& indices)
{
	if (vertices.size() == 0 || indices.size() == 0)
	{
		LoadSucessful = false;
		return;
	}

	VertexBuffers = new RHIBufferGroup();
	IndexBuffers = new RHIBufferGroup();
	VertexBuffers->CreateVertexBuffer(sizeof(OGLVertex), sizeof(OGLVertex)* (int)vertices.size(), EBufferAccessType::Static);
	VertexBuffers->UpdateVertexBuffer(vertices.data(), sizeof(OGLVertex)* vertices.size(), vertices.size());
	IndexBuffers->CreateIndexBuffer(sizeof(IndType), sizeof(IndType)* (int)indices.size());
	IndexBuffers->UpdateIndexBuffer(indices.data(), indices.size());


	//compute AABB for this entity

	glm::vec3 Min = glm::vec3();
	glm::vec3 Max = glm::vec3();
	for (size_t index = 0; index < indices.size(); index++)
	{
		Min = glm::min(Min, vertices[indices[index]].m_position);
		Max = glm::max(Max, vertices[indices[index]].m_position);
	}
	AABB = CullingAABB::CreateFromMinMax(Min, Max);
	LoadSucessful = true;
}

MeshEntity::MeshEntity()
{}

void MeshEntity::InstanceElement(MeshEntity* other, MeshLoader::FMeshLoadingSettings& Settings)
{
	LoadSucessful = other->LoadSucessful;
	VertexBuffers = other->VertexBuffers;
	IndexBuffers = other->IndexBuffers;
	AABB = new CullingAABB(other->AABB->GetPos(), other->AABB->GetHalfExtends_Unscaled());
}
