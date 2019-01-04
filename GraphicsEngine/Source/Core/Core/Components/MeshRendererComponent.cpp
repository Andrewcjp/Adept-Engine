
#include "MeshRendererComponent.h"
#include "CompoenentRegistry.h"
#include "RHI/RHI.h"
#include "RHI/RHICommandList.h"
#include "Core/Assets/Archive.h"
#include "Core/Assets/AssetManager.h"
#include "Editor/Inspector.h"
#include "Component.h"
#include "../GameObject.h"
MeshRendererComponent::MeshRendererComponent()
{
	m_mesh = nullptr;
	TypeID = CompoenentRegistry::BaseComponentTypes::MeshComp;
}

MeshRendererComponent::MeshRendererComponent(Mesh* Mesh, Material* materal) :MeshRendererComponent()
{
	SetUpMesh(Mesh, materal);
}

MeshRendererComponent::~MeshRendererComponent()
{
	//todo: safe mesh removal
}

void MeshRendererComponent::SetUpMesh(Mesh * Mesh, Material * materal)
{
	m_mesh = Mesh;
	SetMaterial(materal, 0);
}

void MeshRendererComponent::Render(bool DepthOnly, RHICommandList* list)
{
	if (m_mesh != nullptr && IsVisible)
	{
		m_mesh->Render(list, !DepthOnly);
	}
}

Material *MeshRendererComponent::GetMaterial(int index)
{
	if (m_mesh == nullptr)
	{
		return nullptr;
	}
	return 	m_mesh->GetMaterial(index);
}

#if WITH_EDITOR
void MeshRendererComponent::GetInspectorProps(std::vector<InspectorProperyGroup>& props)
{
	InspectorProperyGroup group = Inspector::CreatePropertyGroup("Mesh Component");
	group.SubProps.push_back(Inspector::CreateProperty("test", EditValueType::Float, nullptr));
	props.push_back(group);
}
#endif

void MeshRendererComponent::SetMaterial(Material * mat, int index)
{
	if (m_mesh != nullptr)
	{
		m_mesh->SetMaterial(mat, index);
	}
}

void MeshRendererComponent::SetVisiblity(bool state)
{
	IsVisible = state;
}

void MeshRendererComponent::LoadAnimation(std::string filename, std::string name)
{
	LoadAnimation(filename, name, MeshLoader::FMeshLoadingSettings());
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

