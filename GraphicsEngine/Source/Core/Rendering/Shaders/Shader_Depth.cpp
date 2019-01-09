#include "Shader_Depth.h"
#include "RHI/RHI.h"
#include "Shader_Main.h"
IMPLEMENT_GLOBAL_SHADER(Shader_Depth);
Shader_Depth::Shader_Depth(bool LoadGeo) :Shader_Depth(RHI::GetDeviceContext(0), LoadGeo)
{}

Shader_Depth::Shader_Depth(DeviceContext* device, bool LoadGeo) : Shader(device)
{
	LoadGeomShader = LoadGeo;
	m_Shader = RHI::CreateShaderProgam(Device);
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("DIRECTIONAL", LoadGeomShader ? "0" : "1"));
	m_Shader->AttachAndCompileShaderFromFile("depthbasic_vs_12", EShaderType::SHADER_VERTEX);
	if (LoadGeomShader)
	{
		m_Shader->AttachAndCompileShaderFromFile("depthbasic_geo", EShaderType::SHADER_GEOMETRY);
	}
	m_Shader->AttachAndCompileShaderFromFile("depthbasic_fs_12", EShaderType::SHADER_FRAGMENT);
	const int ShadowFarPlane = 500;
	zfar = static_cast<float>(ShadowFarPlane);
	if (RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS > 0)
	{
		ConstantBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant, Device);
		ConstantBuffer->CreateConstantBuffer(sizeof(LightData), RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS, true);
	}
}

void Shader_Depth::UpdateBuffer(RHICommandList * list, LightData* data, int index)
{
	ConstantBuffer->UpdateConstantBuffer(data, index);
	list->SetConstantBufferView(ConstantBuffer, index, Shader_Depth_RSSlots::VPBuffer);
}

Shader_Depth::~Shader_Depth()
{
	EnqueueSafeRHIRelease(ConstantBuffer);
}

std::vector<Shader::ShaderParameter> Shader_Depth::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 0, Shader_Depth_RSSlots::ModelBuffer));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 1, Shader_Depth_RSSlots::GeometryProjections));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 2, Shader_Depth_RSSlots::VPBuffer));
	return Output;
}

