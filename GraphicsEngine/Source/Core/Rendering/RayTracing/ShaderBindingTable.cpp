#include "ShaderBindingTable.h"
#include "Core/Assets/Scene.h"
#include "Rendering/Shaders/Raytracing/Shader_RTMaterialHit.h"
#include "../Core/Mesh.h"
#include "../Core/Material.h"
#include "Core/Utils/StringUtil.h"
#include "RHI/RHIBufferGroup.h"
#include "RayTracingScene.h"

ShaderBindingTable::ShaderBindingTable()
{}

ShaderBindingTable::~ShaderBindingTable()
{}

void ShaderBindingTable::RebuildHittableFromScene(Scene* Sc)
{
	//HitGroups.clear();
	//for (int i = 0; i < Sc->GetMeshObjects().size(); i++)
	//{
	//	AddObject(Sc->GetMeshObjects()[i]);
	//}
}
void ShaderBindingTable::UpdateShaders(Scene* Sc)
{
	//int Index = 0;
	//for (int o = 0; o < Sc->GetMeshObjects().size(); o++)
	//{
	//	GameObject* Object = Sc->GetMeshObjects()[o];
	//	for (int i = 0; i < Object->GetMesh()->SubMeshes.size(); i++)
	//	{
	//		//OnMeshProcessed(Object->GetMesh(), Object->GetMesh()->SubMeshes[i], HitGroups[Index]->HitShader);
	//		Index++;
	//	}
	//}
}

void ShaderBindingTable::AddObject(GameObject* Object)
{
	//for (int i = 0; i < Object->GetMesh()->SubMeshes.size(); i++)
	//{
	//	HitGroups.push_back(new ShaderHitGroup("default"));
	//	HitGroups[HitGroups.size() - 1]->HitShader = GetMaterialShader();
	//	Shader_RTBase* Shader = HitGroups[HitGroups.size() - 1]->HitShader;

	//	Shader->LocalRootSig.SetBufferReadOnly(DefaultLocalRootSignatureParams::IndexBuffer, Object->GetMesh()->SubMeshes[i]->IndexBuffers->Get(0));
	//	Shader->LocalRootSig.SetBufferReadOnly(DefaultLocalRootSignatureParams::VertexBuffer, Object->GetMesh()->SubMeshes[i]->VertexBuffers->Get(0));
	//	//OnMeshProcessed(Object->GetMesh(), Object->GetMesh()->SubMeshes[i], Shader);
	//	if (Object->GetMesh()->GetMaterial(0)->GetRenderPassType() == EMaterialRenderType::Transparent)
	//	{
	//		HitGroups[HitGroups.size() - 1]->AnyHitShader = new Shader_RTBase(RHI::GetDefaultDevice(), "RayTracing\\DefaultAnyHit", ERTShaderType::AnyHit);
	//		//Shader_RTBase* anyhit = HitGroups[HitGroups.size() - 1]->AnyHitShader;
	//	}
	//}
}

Shader_RTBase* ShaderBindingTable::GetMaterialShader()
{
	return new Shader_RTMaterialHit(RHI::GetDefaultDevice());
}

void ShaderBindingTable::InitTable()
{}

void ShaderBindingTable::Validate()
{
	std::map<std::string, int> CountMap;
	ValidateShaderExports(CountMap, MissShaders);
	ValidateShaderExports(CountMap, RayGenShaders);
	/*for (ShaderHitGroup* Group : HitGroups)
	{
		AddToMap(Group->HitShader, CountMap);
	}*/

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

void ShaderBindingTable::ProcessRTSceneEntity( RTSceneEntry * object)
{
	ShaderHitGroupInstance* ins = object->Instance;
	if (ins == nullptr)
	{
		std::string HitName = "default";
		if (object->Object == nullptr)
		{
			HitName = "inter";
		}
		else
		{
			if (object->Object->GetMesh()->GetMaterial(0)->GetRenderPassType() == EMaterialRenderType::Transparent)
			{
				HitName = "anyhit";
			}
		}
		ins = LinkedStateObject->AllocateHitGroupInstance(HitName);
		object->Instance = ins;
		HitGroupInstances.push_back(ins);
	}
	if (ins == nullptr)
	{
		return;
	}
	if (object->E == nullptr)
	{
		return;
	}
	ins->mClosetHitRS.SetBufferReadOnly(DefaultLocalRootSignatureParams::IndexBuffer, object->E->IndexBuffers->Get(0));
	ins->mClosetHitRS.SetBufferReadOnly(DefaultLocalRootSignatureParams::VertexBuffer, object->E->VertexBuffers->Get(0));
	OnMeshProcessed(object->Object->GetMesh(), object->E, ins);
}

void ShaderBindingTable::OnMeshProcessed(Mesh* Mesh, MeshEntity* E, ShaderHitGroupInstance* Shader)
{}

ShaderHitGroup::ShaderHitGroup(std::string name)
{
	Name = name;
	WName = StringUtils::ConvertStringToWide(name);
}

ShaderHitGroupInstance::ShaderHitGroupInstance(ShaderHitGroup * group)
{
	Name = group->Name;
	WName = StringUtils::ConvertStringToWide(Name);
	if (group->HitShader != nullptr)
	{
		mClosetHitRS = group->HitShader->LocalRootSig;
	}
	if (group->AnyHitShader != nullptr)
	{
		mAnyHitRS = group->AnyHitShader->LocalRootSig;
	}
	if (group->IntersectionShader != nullptr)
	{
		mintersectionRS = group->IntersectionShader->LocalRootSig;
	}
}
