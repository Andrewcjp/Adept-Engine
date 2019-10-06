#include "ReflectionsBindingTable.h"
#include "Rendering/Core/Material.h"
#include "Rendering/Core/Mesh.h"
#include "Rendering/RayTracing/Shader_RTBase.h"
#include "Rendering/Shaders/Raytracing/Reflections/Shader_ReflectionRaygen.h"
#include "Rendering/Shaders/Raytracing/Shader_RTMateralHit.h"
#include "Rendering/Shaders/Raytracing/Shader_Skybox_Miss.h"

ReflectionsBindingTable::ReflectionsBindingTable()
{}


ReflectionsBindingTable::~ReflectionsBindingTable()
{}

void ReflectionsBindingTable::InitTable()
{
	//default debug
	MissShaders.push_back(ShaderComplier::GetShader<Shader_Skybox_Miss>());

	RayGenShaders.push_back(ShaderComplier::GetShader<Shader_ReflectionRaygen>());

	HitGroups.push_back(new ShaderHitGroup("HitGroup0"));
	HitGroups[0]->HitShader = new Shader_RTMateralHit(RHI::GetDefaultDevice());
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
	GlobalRootSig.Params.push_back(s);
}

void ReflectionsBindingTable::OnMeshProcessed(Mesh* Mesh, MeshEntity* E, Shader_RTBase* Shader)
{
	Shader->LocalRootSig.SetTexture(2, Mesh->GetMaterial(0)->GetTexturebind("DiffuseMap"));
}
