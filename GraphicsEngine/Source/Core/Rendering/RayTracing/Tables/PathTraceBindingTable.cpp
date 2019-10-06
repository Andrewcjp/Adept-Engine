#include "PathTraceBindingTable.h"
#include "Core\Assets\ShaderComplier.h"
#include "..\Shader_RTBase.h"
#include "..\..\Shaders\Raytracing\Shader_RTMateralHit.h"
#include "..\..\Shaders\Raytracing\Shader_Skybox_Miss.h"
#include "..\..\Core\Mesh.h"
#include "..\..\Core\Material.h"


PathTraceBindingTable::PathTraceBindingTable()
{}


PathTraceBindingTable::~PathTraceBindingTable()
{}

void PathTraceBindingTable::InitTable()
{
	//default debug
	MissShaders.push_back(ShaderComplier::GetShader<Shader_Skybox_Miss>());

	RayGenShaders.push_back(new Shader_RTBase(RHI::GetDefaultDevice(), "Raytracing\\DefaultRayGenShader", ERTShaderType::RayGen));
	RayGenShaders[0]->AddExport("rayGen");

	HitGroups.push_back(new ShaderHitGroup("HitGroup0"));
	HitGroups[0]->HitShader = new Shader_RTMateralHit(RHI::GetDefaultDevice());

	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::UAV, GlobalRootSignatureParams::OutputViewSlot, 0));
	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::RootSRV, GlobalRootSignatureParams::AccelerationStructureSlot, 0));
	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::CBV, 2, 0));

}

void PathTraceBindingTable::OnMeshProcessed(Mesh* Mesh, MeshEntity* E, Shader_RTBase* Shader)
{
	Shader->LocalRootSig.SetTexture(2, Mesh->GetMaterial(0)->GetTexturebind("DiffuseMap"));
}
