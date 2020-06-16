#include "PathTraceBindingTable.h"
#include "Core\Assets\ShaderCompiler.h"
#include "..\Shader_RTBase.h"
#include "..\..\Shaders\Raytracing\Shader_RTMaterialHit.h"
#include "..\..\Shaders\Raytracing\Shader_Skybox_Miss.h"
#include "..\..\Core\Mesh.h"
#include "..\..\Core\Material.h"
#include "Rendering\Core\Defaults.h"
#include "RHI\Streaming\TextureStreamingCommon.h"


PathTraceBindingTable::PathTraceBindingTable()
{}


PathTraceBindingTable::~PathTraceBindingTable()
{}

void PathTraceBindingTable::InitTable()
{
	//default debug
	MissShaders.push_back(ShaderCompiler::GetShader<Shader_Skybox_Miss>());

	RayGenShaders.push_back(new Shader_RTBase(RHI::GetDefaultDevice(), "Raytracing\\DefaultRayGenShader", ERTShaderType::RayGen));
	RayGenShaders[0]->AddExport("rayGen");

	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::UAV, GlobalRootSignatureParams::OutputViewSlot, 0));
	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::RootSRV, GlobalRootSignatureParams::AccelerationStructureSlot, 0));
	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::CBV, 2, 0));

}

Shader_RTBase* PathTraceBindingTable::GetMaterialShader()
{
	return ShaderBindingTable::GetMaterialShader();
	Shader_RTBase* out = new Shader_RTBase(RHI::GetDefaultDevice(), "Raytracing\\Pathtrace\\Path_MaterialHitShader", ERTShaderType::Hit);
	out->AddExport("chs");
	return out;
}

void PathTraceBindingTable::OnMeshProcessed(Mesh* Mesh, MeshEntity* E, ShaderHitGroupInstance* Shader)
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
