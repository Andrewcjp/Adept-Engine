#include "ReflectionsBindingTable.h"
#include "Rendering/Core/Material.h"
#include "Rendering/Core/Mesh.h"
#include "Rendering/RayTracing/Shader_RTBase.h"
#include "Rendering/Shaders/Raytracing/Reflections/Shader_ReflectionRaygen.h"
#include "Rendering/Shaders/Raytracing/Shader_RTMaterialHit.h"
#include "Rendering/Shaders/Raytracing/Shader_Skybox_Miss.h"
#include "RHI/Streaming/TextureStreamingCommon.h"
#include "Rendering/Core/Defaults.h"

ReflectionsBindingTable::ReflectionsBindingTable()
{}


ReflectionsBindingTable::~ReflectionsBindingTable()
{}

void ReflectionsBindingTable::InitTable()
{
	//default debug
	MissShaders.push_back(ShaderComplier::GetShader<Shader_Skybox_Miss>());

	RayGenShaders.push_back(ShaderComplier::GetShader<Shader_ReflectionRaygen>());


	//HitGroups[0]->AnyHitShader = new Shader_RTBase(RHI::GetDefaultDevice(), "RayTracing\\DefaultAnyHit", ERTShaderType::AnyHit);
	//HitGroups[0]->AnyHitShader->AddExport("anyhit_main");
	//HitGroups[0]->AnyHitShader->InitRS();

	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::UAV, GlobalRootSignatureParams::OutputViewSlot, 0));
	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::RootSRV, GlobalRootSignatureParams::AccelerationStructureSlot, 0));
	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::CBV, 2, 0));
	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::SRV, 3, 5));
	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::SRV, 4, 6));
	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::CBV, 5, 1));
	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::SRV, 6, 20));
	ShaderParameter s = ShaderParameter(ShaderParamType::SRV, 7, 5, 2);
	s.NumDescriptors = RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS;
	GlobalRootSig.Params.push_back(s);
	s = ShaderParameter(ShaderParamType::RootConstant, 8, 2);
	s.NumDescriptors = 2;
	s.NumVariablesContained = 2;
	GlobalRootSig.Params.push_back(s);
	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::SRV, 9, 66));
	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::CBV, 10, 3));
	//GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::SRV, 11, 10));
}

void ReflectionsBindingTable::OnMeshProcessed(Mesh* Mesh, MeshEntity* E, ShaderHitGroupInstance* Shader)
{
	Mesh->GetMaterial(0)->ParmbindSet.MakeActive();
	if (Mesh->GetMaterial(0)->GetTexture(0) == nullptr)
	{
		Shader->mClosetHitRS.SetTexture2(2, Defaults::GetDefaultTexture2(), RHIViewDesc::DefaultSRV());
		return;
	}
	if (Mesh->GetMaterial(0)->GetTexture(0)->GetData(RHI::GetDefaultDevice())->Backing == nullptr)
	{
		Shader->mClosetHitRS.SetTexture2(2, Defaults::GetDefaultTexture2(), RHIViewDesc::DefaultSRV());
		return;
	}
	Shader->mClosetHitRS.SetTexture2(2, Mesh->GetMaterial(0)->GetTexture(0)->GetData(RHI::GetDefaultDevice())->Backing, Mesh->GetMaterial(0)->GetTexture(0)->GetCurrentView(0));
}
