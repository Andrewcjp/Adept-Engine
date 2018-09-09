#include "Shader_Depth.h"
#include "RHI/RHI.h"
#include "glm\glm.hpp"
#include "Shader_Main.h"

Shader_Depth::Shader_Depth(bool LoadGeo) :Shader_Depth(LoadGeo, RHI::GetDeviceContext(0))
{

}
Shader_Depth::Shader_Depth(bool LoadGeo, DeviceContext* device)
{
	LoadGeomShader = LoadGeo;
	m_Shader = RHI::CreateShaderProgam(device);

	if (!LoadGeomShader)
	{
		m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("DIRECTIONAL", "1"));
	}

	m_Shader->AttachAndCompileShaderFromFile("depthbasic_vs_12", EShaderType::SHADER_VERTEX);
	if (LoadGeomShader)
	{
		m_Shader->AttachAndCompileShaderFromFile("depthbasic_geo", EShaderType::SHADER_GEOMETRY);
	}
	m_Shader->AttachAndCompileShaderFromFile("depthbasic_fs_12", EShaderType::SHADER_FRAGMENT);


	zfar = static_cast<float>(ShadowFarPlane);
	if (RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS > 0)
	{
		ConstantBuffer = RHI::CreateRHIBuffer(RHIBuffer::BufferType::Constant, device);
		ConstantBuffer->CreateConstantBuffer(sizeof(LightData), RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS);
	}
}


void Shader_Depth::UpdateBuffer(RHICommandList * list, LightData* data, int index)
{
	ConstantBuffer->UpdateConstantBuffer(data, index);
	list->SetConstantBufferView(ConstantBuffer, index, 2);
}



Shader_Depth::~Shader_Depth()
{

}

void Shader_Depth::SetShaderActive()
{}

std::vector<Shader::ShaderParameter> Shader_Depth::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 0, Shader_Depth_RSSlots::ModelBuffer));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 1, Shader_Depth_RSSlots::GeometryProjections));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 2, Shader_Depth_RSSlots::VPBuffer));
	//Shader_Main::GetMainShaderSig(Output);
	return Output;
}

