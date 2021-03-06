
#include "MeshRendererComponent.h"

#include "Core/Assets/Archive.h"
#include "Core/Assets/AssetManager.h"
#include "Rendering/RayTracing/LowLevelAccelerationStructure.h"
#include "Rendering/RayTracing/RayTracingEngine.h"
#include "Editor/UIInspectorBase.h"
#include "../GameObject.h"
#include "Core/Assets/BinaryArchive.h"
#include "Core/Assets/Asset types/MaterialAsset.h"
#include "Core/Components/MeshRendererComponent.generated.h"

MeshRendererComponent::MeshRendererComponent()
{
	m_mesh = nullptr;
	m_DisplayName = "Mesh Renderer Component";
	CALL_CONSTRUCTOR();
}

MeshRendererComponent::MeshRendererComponent(Mesh* Mesh, Material* materal) :MeshRendererComponent()
{
	SetUpMesh(Mesh, materal);
}

MeshRendererComponent::~MeshRendererComponent()
{
	//todo: safe mesh removal
	EnqueueSafeRHIRelease(m_mesh);
	MemoryUtils::DeleteReleaseableVector(MeshAcclerations);
}

void MeshRendererComponent::SetUpMesh(Mesh * Mesh, Material * materal)
{
	m_mesh = Mesh;
	AssetPath = m_mesh->AssetName;
	m_mesh->Renderer = this;
	SetMaterial(materal, 0);
}


Material *MeshRendererComponent::GetMaterial(int index)
{
	if (m_mesh == nullptr)
	{
		return nullptr;
	}
	return 	m_mesh->GetMaterial(index);
}

Mesh* MeshRendererComponent::GetMesh()
{
	return m_mesh;
}

void MeshRendererComponent::SetMaterial(Material * mat, int index)
{
	if (m_mesh != nullptr)
	{
		m_mesh->SetMaterial(mat, index);
		m_mesh->InvalidateBatch();
	}
}

void MeshRendererComponent::SetVisiblity(bool state)
{
	IsVisible = state;
	m_mesh->IsVisible = state;
}

void MeshRendererComponent::LoadAnimation(std::string filename, std::string name)
{
	MeshLoader::FMeshLoadingSettings t = MeshLoader::FMeshLoadingSettings();
	LoadAnimation(filename, name, t);
}

void MeshRendererComponent::LoadAnimation(std::string filename, std::string name, MeshLoader::FMeshLoadingSettings& Settings)
{
	if (m_mesh == nullptr || m_mesh->GetSkeletalMesh() == nullptr)
	{
		return;
	}
	std::string accpath = AssetManager::GetContentPath();
	accpath.append(filename);
	MeshLoader::LoadAnimOnly(accpath, m_mesh->GetSkeletalMesh(), name, Settings);
}

void MeshRendererComponent::PlayAnim(std::string name)
{
	if (m_mesh != nullptr && m_mesh->GetSkeletalMesh() != nullptr)
	{
		m_mesh->GetSkeletalMesh()->PlayAnimation(name);
	}
}

glm::vec3 MeshRendererComponent::GetPosOfBone(std::string Name)
{
	glm::vec3 LocalSpacePos = GetMesh()->GetPosOfBone(Name);
	glm::mat4x4 Model = (GetOwner()->GetTransform()->GetModel());
	//LocalSpacePos = Model * glm::vec4(LocalSpacePos, 1.0f);
	return LocalSpacePos;
}

void MeshRendererComponent::PrepareDataForRender()
{
	m_mesh->PrepareDataForRender(GetOwner());
}

void MeshRendererComponent::SceneInitComponent()
{
	if (m_mesh == nullptr)
	{
		return;
	}
	m_mesh->UpdateBounds(GetOwner()->GetPosition(), GetOwner()->GetTransform()->GetScale());
	if (RHI::GetRenderSettings()->RaytracingEnabled())
	{
		for (int i = 0; i < m_mesh->SubMeshes.size(); i++)
		{
			LowLevelAccelerationStructure* BLAS = RHI::GetRHIClass()->CreateLowLevelAccelerationStructure(RHI::GetDefaultDevice(), AccelerationStructureDesc());
			BLAS->CreateFromEntity(m_mesh->SubMeshes[i]);
			BLAS->LayerMask = GetOwner()->LayerMask.GetFlags();
			BLAS->UpdateTransfrom(GetOwner()->GetTransform());
			RayTracingEngine::Get()->EnqueueForBuild(BLAS);
			MeshAcclerations.push_back(BLAS);
		}
	}
	if (m_mesh != nullptr)
	{
		m_mesh->GetMaterial(0)->MakeReady();
	}
}

void MeshRendererComponent::OnTransformUpdate()
{
	if (m_mesh == nullptr)
	{
		return;
	}
	m_mesh->UpdateBounds(GetOwner()->GetPosition(), GetOwner()->GetTransform()->GetScale());
}

LowLevelAccelerationStructure * MeshRendererComponent::GetAccelerationStructure() const
{
	return nullptr;
}

void MeshRendererComponent::SetMaterialAsset(std::string path)
{
	m_pMaterialAsset.SetAsset(path);
	if (m_pMaterialAsset.IsValid())
	{
		SetMaterial(m_pMaterialAsset.GetAsset()->CreateMaterial(), 0);
	}
}

void MeshRendererComponent::Serialize(BinaryArchive* Achive)
{
	SerializeThis(Achive, m_RelfectionData.Data);
	if (AssetPath.length() > 0 && Achive->Reading)
	{
		MeshLoader::FMeshLoadingSettings set;
		m_mesh = RHI::CreateMesh(AssetPath.c_str(), set);
		m_mesh->Renderer = this;
	}
}

void MeshRendererComponent::OnPropertyUpdate(ClassReflectionNode* Node)
{
	if (Node->m_MemberName == "m_pMaterialAsset")
	{
		if (m_pMaterialAsset.IsValid())
		{
			SetMaterial(m_pMaterialAsset.GetAsset()->CreateMaterial(), 0);
		}
	}
}

void MeshRendererComponent::BeginPlay()
{}

void MeshRendererComponent::Update(float dt)
{
	if (m_mesh != nullptr)
	{
		m_mesh->Tick(dt);
		if (m_mesh->GetSkeletalMesh())
		{
			//	m_mesh->GetSkeletalMesh()->RenderBones(GetOwner()->GetTransform());
		}
	}
}

void MeshRendererComponent::InitComponent()
{}

void MeshRendererComponent::ProcessSerialArchive(Archive * A)
{
	Component::ProcessSerialArchive(A);
	if (A->IsReading())
	{
		std::string Assetname;
		ArchiveProp_Alias(Assetname, m_mesh->AssetName);
		MeshLoader::FMeshLoadingSettings set;
		set.Serialize(A);
		if (!Assetname.empty())
		{
			m_mesh = RHI::CreateMesh(Assetname.c_str(), set);
			m_mesh->Renderer = this;
		}
		else
		{
			LogEnsure_Always("Failed to load mesh for node");
		}
	}
	else
	{
		if (m_mesh != nullptr)
		{
			ArchiveProp(m_mesh->AssetName);
			m_mesh->ImportSettings.Serialize(A);
		}
	}
	m_mesh->ProcessSerialArchive(A);
}

