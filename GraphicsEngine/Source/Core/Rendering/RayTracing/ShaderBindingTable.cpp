#include "ShaderBindingTable.h"
#include "Core/Assets/Scene.h"
#include "Rendering/Shaders/Raytracing/Shader_RTMateralHit.h"
#include "../Core/Mesh.h"
#include "../Core/Material.h"
#include "Core/Utils/StringUtil.h"

ShaderBindingTable::ShaderBindingTable()
{}

ShaderBindingTable::~ShaderBindingTable()
{}

void ShaderBindingTable::RebuildHittableFromScene(Scene* Sc)
{
	HitGroups.clear();
	for (int i = 0; i < Sc->GetMeshObjects().size(); i++)
	{
		AddObject(Sc->GetMeshObjects()[i]);
	}
}

void ShaderBindingTable::AddObject(GameObject* Object)
{
	for (int i = 0; i < Object->GetMesh()->SubMeshes.size(); i++)
	{
		HitGroups.push_back(new ShaderHitGroup("HitGroup0"));
		HitGroups[HitGroups.size() - 1]->HitShader = new Shader_RTMateralHit(RHI::GetDefaultDevice());
		Shader_RTBase* Shader = HitGroups[HitGroups.size() - 1]->HitShader;

		Shader->LocalRootSig.SetBufferReadOnly(DefaultLocalRootSignatureParams::IndexBuffer, Object->GetMesh()->SubMeshes[i]->IndexBuffers[0].Get());
		Shader->LocalRootSig.SetBufferReadOnly(DefaultLocalRootSignatureParams::VertexBuffer, Object->GetMesh()->SubMeshes[i]->VertexBuffers[0].Get());
		OnMeshProcessed(Object->GetMesh(), Object->GetMesh()->SubMeshes[i], Shader);
		if (Object->GetMesh()->GetMaterial(0)->GetRenderPassType() == EMaterialRenderType::Transparent)
		{
			HitGroups[HitGroups.size() - 1]->AnyHitShader = new Shader_RTBase(RHI::GetDefaultDevice(), "RayTracing\\DefaultAnyHit", ERTShaderType::AnyHit);
			//Shader_RTBase* anyhit = HitGroups[HitGroups.size() - 1]->AnyHitShader;
		}
	}
}

void ShaderBindingTable::InitTable()
{}

void ShaderBindingTable::Validate()
{
	std::map<std::string, int> CountMap;
	ValidateShaderExports(CountMap, MissShaders);
	ValidateShaderExports(CountMap, RayGenShaders);
	for (ShaderHitGroup* Group : HitGroups)
	{
		AddToMap(Group->HitShader, CountMap);
	}

	for (auto it = CountMap.begin(); it != CountMap.end(); ++it)
	{
		ensureMsgf(it->second < 2, "Duplicated export found");
	}
}

void ShaderBindingTable::ValidateShaderExports(std::map<std::string, int>& CountMap, std::vector<Shader_RTBase*>& shaders)
{
	for (int i = 0; i < shaders.size(); i++)
	{
		AddToMap(shaders[i], CountMap);
	}
}

void ShaderBindingTable::AddToMap(Shader_RTBase * shader, std::map<std::string, int>& CountMap)
{
	for (auto it = shader->GetExports().begin(); it != shader->GetExports().end(); ++it)
	{
		CountMap[*it]++;
	}
}

void ShaderBindingTable::OnMeshProcessed(Mesh* Mesh, MeshEntity* E, Shader_RTBase* Shader)
{}

ShaderHitGroup::ShaderHitGroup(std::string name)
{
	Name = name;
	WName = StringUtils::ConvertStringToWide(name);
}
