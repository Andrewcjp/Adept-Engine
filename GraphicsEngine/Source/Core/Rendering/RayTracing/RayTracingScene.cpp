#include "RayTracingScene.h"
#include "RTAccelerationStructureCache.h"
#include "Core/GameObject.h"
#include "Core/Components/MeshRendererComponent.h"
#include "LowLevelAccelerationStructure.h"
#include "RHI/RHITimeManager.h"
#include "HighLevelAccelerationStructure.h"
#include "Shader_RTBase.h"
#define TEST_INTERSECT 0

void RayTracingScene::Create(Scene * scene)
{
#if TEST_INTERSECT
	LowLevelAccelerationStructure* Structure = RHI::GetRHIClass()->CreateLowLevelAccelerationStructure(RHI::GetDefaultDevice(), AccelerationStructureDesc());
	std::vector<RTAABB> list;
	const float size = 1.0f;
	list.push_back(RTAABB(glm::vec3(size, size, size), -glm::vec3(size, size, size)));
	//list.push_back(RTAABB(glm::vec3(0, 10, 0) + glm::vec3(size, size, size),-glm::vec3(size, size, size)));
	Structure->CreateFromAABBList(list);
	RTSceneEntry* entry = new RTSceneEntry();
	entry->CurrnetState = ERTSceneEntityState::NeedsBuild;
	entry->Object = nullptr;
	entry->Structure = Structure;
	//entry->E = object->GetMesh()->SubMeshes[i];
	RTObjects.push_back(entry);
#endif
}

void RayTracingScene::Update()
{
	for (int i = 0; i < RTObjects.size(); i++)
	{
		//RTObjects[i]->Object = 
	}
	ConsiderObjects();
}

RayTracingScene::RayTracingScene()
{
	mCache = new RTAccelerationStructureCache();
	AccelerationStructureDesc Desc;
	Desc.MaxSize = 250;
	mTopLevel = RHI::GetRHIClass()->CreateHighLevelAccelerationStructure(RHI::GetDefaultDevice(), Desc);
	mTopLevel->InitialBuild();
}

RayTracingScene::~RayTracingScene()
{}

void RayTracingScene::ConsiderObjects()
{
	ActiveEntitys.clear();
	ActiveEntitys_NeedBuild.clear();
	for (int i = 0; i < RTObjects.size(); i++)
	{
		if (RTObjects[i]->CurrnetState == ERTSceneEntityState::NeedsBuild)
		{
			ActiveEntitys_NeedBuild.push_back(RTObjects[i]);
		}
		else
		{
			ActiveEntitys.push_back(RTObjects[i]);
		}
	}
}

void RayTracingScene::UpdateStateObject(ShaderBindingTable* table)
{
	if (SceneStateObject != nullptr)
	{
		//todo: check current shaders are included
		return;
	}
	RHIStateObjectDesc Desc;
	Desc.AttibuteSize = sizeof(glm::vec2);// float2 barycentrics
	Desc.PayloadSize = sizeof(glm::vec4) * 3;    // float4 pixelColor
	Desc.ShaderRecords.GlobalRS = table->GlobalRootSig;
	Desc.ShaderRecords.MissShaders = table->MissShaders;
	Desc.ShaderRecords.RayGenShaders = table->RayGenShaders;
	//for (int i = 0; i < RTObjects.size(); i++)
	//{
	ShaderHitGroup* hit = new ShaderHitGroup("default");
	hit->HitShader = table->GetMaterialShader();
	Desc.ShaderRecords.HitGroups.push_back(hit);

	hit = new ShaderHitGroup("anyhit");
	hit->HitShader = table->GetMaterialShader();
	hit->AnyHitShader = new Shader_RTBase(RHI::GetDefaultDevice(), "RayTracing\\DefaultAnyHit", ERTShaderType::AnyHit);
	hit->AnyHitShader->AddExport("anyhit_main");
	Desc.ShaderRecords.HitGroups.push_back(hit);
	//}
#if TEST_INTERSECT
	hit = new ShaderHitGroup("inter");
	hit->HitShader = new Shader_RTBase(RHI::GetDefaultDevice(), "RayTracing\\DefaultHitShader", ERTShaderType::Hit);
	hit->HitShader->AddExport("chsdefault");
	hit->IntersectionShader = new Shader_RTBase(RHI::GetDefaultDevice(), "RayTracing\\DefaultInterSection", ERTShaderType::Intersection);
	hit->IntersectionShader->AddExport("intersection_main");
	hit->GroupType = RHIHIT_GROUP_TYPE::HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;
	Desc.ShaderRecords.HitGroups.push_back(hit);
#endif

	SceneStateObject = RHI::GetRHIClass()->CreateStateObject(RHI::GetDefaultDevice(), Desc);
	SceneStateObject->Build();
	table->LinkedStateObject = SceneStateObject;
	//for now update table here
	for (int i = 0; i < ActiveEntitys.size(); i++)
	{
		table->ProcessRTSceneEntity(ActiveEntitys[i]);
	}
}

void RayTracingScene::UpdateShaderTable(ShaderBindingTable* table)
{
	UpdateStateObject(table);
	SceneStateObject->ShaderTable = table;
	for (int i = 0; i < ActiveEntitys.size(); i++)
	{
		table->ProcessRTSceneEntity(ActiveEntitys[i]);
	}
	SceneStateObject->RebuildShaderTable();
}

void RayTracingScene::ProcessUpdatesToAcclerationStuctures(RHICommandList * list)
{
	{
		DECALRE_SCOPEDGPUCOUNTER(list, "RT.Build.Instances");
		for (int i = 0; i < ActiveEntitys_NeedBuild.size(); i++)
		{
			ActiveEntitys_NeedBuild[i]->Structure->Build(list);
			ActiveEntitys_NeedBuild[i]->CurrnetState = ERTSceneEntityState::Active;
			ActiveEntitys.push_back(ActiveEntitys_NeedBuild[i]);
		}
	}
	{
		DECALRE_SCOPEDGPUCOUNTER(list, "RT.Build.TopLevel");
		mTopLevel->ClearInstances();
		for (int i = 0; i < ActiveEntitys.size(); i++)
		{
			AccelerationStructureInstanceDesc Idesc;
			if (ActiveEntitys[i]->Object != nullptr)
			{
				Idesc.Transform = ActiveEntitys[i]->Object->GetTransform()->GetModel();
			}
			else
			{
				Idesc.Transform = glm::mat4(1);
			}
			Idesc.HitGroupOffset = i;
			Idesc.Structure = ActiveEntitys[i]->Structure;
			mTopLevel->AddInstance(Idesc);
		}
		mTopLevel->Build(list);
	}
}

void RayTracingScene::OnObjectAddedToScene(GameObject * object)
{
	for (int i = 0; i < object->GetMesh()->SubMeshes.size(); i++)
	{
		LowLevelAccelerationStructure* Structure = mCache->CreateStructure(object->GetMesh()->SubMeshes[i], object->GetMesh(), i);
		RTSceneEntry* entry = new RTSceneEntry();
		entry->CurrnetState = Structure->IsDirty() ? ERTSceneEntityState::NeedsBuild : ERTSceneEntityState::Inactive;
		entry->Object = object;
		entry->Structure = Structure;
		entry->E = object->GetMesh()->SubMeshes[i];
		RTObjects.push_back(entry);

	}
}
