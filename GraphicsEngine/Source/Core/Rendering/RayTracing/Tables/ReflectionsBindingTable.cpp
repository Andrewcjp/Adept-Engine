#include "ReflectionsBindingTable.h"
#include "..\Shader_RTBase.h"
#include "..\..\Core\Mesh.h"
#include "..\..\Core\Material.h"
#include "..\..\Shaders\Raytracing\Shader_Skybox_Miss.h"
#include "..\..\Shaders\Raytracing\Reflections\Shader_ReflectionRaygen.h"
#include "..\..\Shaders\Raytracing\Shader_RTMateralHit.h"


ReflectionsBindingTable::ReflectionsBindingTable()
{}


ReflectionsBindingTable::~ReflectionsBindingTable()
{}

void ReflectionsBindingTable::InitTable()
{
	//default debug
	MissShaders.push_back(ShaderComplier::GetShader<Shader_Skybox_Miss>());
	if (MissShaders[0]->GetExports().size() == 0)
	{
		MissShaders[0]->AddExport("Miss");
	}

	RayGenShaders.push_back(ShaderComplier::GetShader<Shader_ReflectionRaygen>());

	HitGroups.push_back(new ShaderHitGroup("HitGroup0"));
	HitGroups[0]->HitShader = new Shader_RTMateralHit(RHI::GetDefaultDevice());

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
}

void ReflectionsBindingTable::OnMeshProcessed(Mesh* Mesh, MeshEntity* E, Shader_RTBase* Shader)
{
	Shader->LocalRootSig.SetTexture(2, Mesh->GetMaterial(0)->GetTexturebind("DiffuseMap"));
}
