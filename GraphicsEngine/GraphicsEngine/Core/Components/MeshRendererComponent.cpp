#include "stdafx.h"
#include "MeshRendererComponent.h"


MeshRendererComponent::MeshRendererComponent(Renderable* Mesh, Material* materal)
{
	m_mesh = Mesh;
	m_mat = materal;
}


MeshRendererComponent::~MeshRendererComponent()
{
	delete m_mat;
	delete m_mesh;
}

void MeshRendererComponent::Render(bool DepthOnly)
{
	if (m_mat != nullptr && DepthOnly == false)
	{
		m_mat->SetMaterialActive();
	}
	m_mesh->Render();
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
{
}

void MeshRendererComponent::Update(float delta)
{
}
