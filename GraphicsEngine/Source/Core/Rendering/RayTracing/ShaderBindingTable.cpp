#include "ShaderBindingTable.h"
#include "Shader_RTBase.h"
#include "../Shaders/Raytracing/Shader_Skybox_Miss.h"


ShaderBindingTable::ShaderBindingTable()
{
	InitDefault();
}

void ShaderBindingTable::InitDefault()
{
	//default debug
	MissShaders.push_back(ShaderComplier::GetShader<Shader_Skybox_Miss>());
	MissShaders[0]->AddExport("Miss");

	RayGenShaders.push_back(new Shader_RTBase(RHI::GetDefaultDevice(), "Raytracing\\DefaultRayGenShader", ERTShaderType::RayGen));
	RayGenShaders[0]->AddExport("rayGen");

	HitGroups.push_back(new ShaderHitGroup("HitGroup1"));
	HitGroups[0]->HitShader = new Shader_RTBase(RHI::GetDefaultDevice(), "Raytracing\\DefaultHitShader", ERTShaderType::Hit);
	HitGroups[0]->HitShader->AddExport("chs");

	//rootParameters[GlobalRootSignatureParams::OutputViewSlot].InitAsDescriptorTable(1, &ranges[0]);
	//rootParameters[GlobalRootSignatureParams::AccelerationStructureSlot].InitAsShaderResourceView(0);
	//rootParameters[GlobalRootSignatureParams::CameraBuffer].InitAsConstantBufferView(0);
	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::UAV, 0, 0));
	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::RootSRV, 1, 0));
	GlobalRootSig.Params.push_back(ShaderParameter(ShaderParamType::CBV, 2, 0));
}

ShaderBindingTable::~ShaderBindingTable()
{}
