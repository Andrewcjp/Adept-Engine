#include "stdafx.h"
#include "MeshRendererComponent.h"
#include "CompoenentRegistry.h"
#include "Core/Assets/SerialHelpers.h"
#include "Core/Assets/SceneJSerialiser.h"
#include "RHI/RHI.h"
#include "RHI/RHICommandList.h"
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

void MeshRendererComponent::Serialise(rapidjson::Value & v)
{
	Component::Serialise(v);
	SerialHelpers::addString(v, *SceneJSerialiser::jallocator, "MeshName", m_mesh->AssetName);
	if (m_mat != nullptr)
	{
#if 0
		//todo: Serial
		if (m_mat->GetDiffusetexture())
		{
			SerialHelpers::addString(v, *SceneJSerialiser::jallocator, "MatDiffuse", m_mat->GetDiffusetexture()->TextureName);
		}
		if (m_mat->GetNormalMap())
		{
			SerialHelpers::addString(v, *SceneJSerialiser::jallocator, "MatNormal", m_mat->GetNormalMap()->TextureName);
		}
		if (m_mat->GetDisplacementMap())
		{
			SerialHelpers::addString(v, *SceneJSerialiser::jallocator, "DisplacementMap", m_mat->GetDisplacementMap()->TextureName);
		}
#endif
	}	//todo:
}

void MeshRendererComponent::Deserialise(rapidjson::Value & v)
{
	for (auto& it = v.MemberBegin(); it != v.MemberEnd(); it++)
	{
		std::string key = (it->name.GetString());
		if (key == "MeshName")
		{
			std::string path = (it->value.GetString());
			if (path.length() != 0)
			{
				//this is not handled for D3D11
				m_mesh = RHI::CreateMesh(path.c_str());
			}
		}
		if (key == "MatDiffuse")
		{
			std::string path = (it->value.GetString());
			if (path.length() != 0)
			{
				m_mat = new Material(RHI::CreateTexture(path));
			}
		}
		if (key == "MatNormal")
		{
			std::string path = (it->value.GetString());
			if (path.length() != 0)
			{
				if (m_mat != nullptr)
				{
					m_mat->SetNormalMap(RHI::CreateTexture(path));
				}
			}
		}
		if (key == "DisplacementMap")
		{
			std::string path = (it->value.GetString());
			if (path.length() != 0)
			{
				if (m_mat != nullptr)
				{
					m_mat->SetDisplacementMap(RHI::CreateTexture(path));
				}
			}
		}

	}
}
