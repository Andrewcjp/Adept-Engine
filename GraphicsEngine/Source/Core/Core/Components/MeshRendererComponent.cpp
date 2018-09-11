#include "stdafx.h"
#include "MeshRendererComponent.h"
#include "CompoenentRegistry.h"
#include "RHI/RHI.h"
#include "RHI/RHICommandList.h"
#include "Core/Assets/Archive.h"
MeshRendererComponent::MeshRendererComponent()
{
	m_mesh = nullptr;
	m_mat = nullptr;
	TypeID = CompoenentRegistry::BaseComponentTypes::MeshComp;
}

MeshRendererComponent::MeshRendererComponent(Renderable* Mesh, Material* materal) :MeshRendererComponent()
{
	SetUpMesh(Mesh, materal);
}

MeshRendererComponent::~MeshRendererComponent()
{
	delete m_mat;
	delete m_mesh;
}

void MeshRendererComponent::SetUpMesh(Renderable * Mesh, Material * materal)
{
	m_mesh = Mesh;
	m_mat = materal;
}

void MeshRendererComponent::Render(bool DepthOnly, RHICommandList* list)
{
	if (m_mat != nullptr && !DepthOnly)
	{
		m_mat->SetMaterialActive(list);
	}
	if (m_mesh != nullptr)
	{
		m_mesh->Render(list);
	}
}

Material *MeshRendererComponent::GetMaterial()
{
	return m_mat;
}

void MeshRendererComponent::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props)
{
	Inspector::InspectorProperyGroup group = Inspector::CreatePropertyGroup("Mesh Component");
	group.SubProps.push_back(Inspector::CreateProperty("test", Inspector::Float, nullptr));
	props.push_back(group);
}

void MeshRendererComponent::BeginPlay()
{}

void MeshRendererComponent::Update(float)
{}

void MeshRendererComponent::InitComponent()
{}

void MeshRendererComponent::ProcessSerialArchive(Archive * A)
{
	Component::ProcessSerialArchive(A);
	if (m_mesh != nullptr)
	{
		ArchiveProp(m_mesh->AssetName);
	}
}

