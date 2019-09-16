#include "Shader_Depth.h"
#include "RHI/RHI.h"
#include "../Shader_Main.h"
#include "Rendering/Core/ShadowRenderer.h"
IMPLEMENT_GLOBAL_SHADER(Shader_Depth);
Shader_Depth::Shader_Depth(bool LoadGeo) :Shader_Depth(RHI::GetDeviceContext(0), LoadGeo)
{}

Shader_Depth::Shader_Depth(DeviceContext* device, bool LoadGeo) : Shader(device)
{
	LoadGeomShader = LoadGeo;
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_INSTANCES", std::to_string(RHI::GetRenderConstants()->MAX_MESH_INSTANCES)));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("WITH_INSTANCING", RHI::GetRenderSettings()->AllowMeshInstancing ? "1" : "0"));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("DIRECTIONAL", LoadGeomShader ? "0" : "1"));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("VS_WORLD_OUTPUT", RHI::GetRenderSettings()->GetShadowSettings().UseGeometryShaderForShadows ? "0" : "1"));
	if (RHI::GetRenderSettings()->GetShadowSettings().UseViewInstancingForShadows)
	{
		m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("USE_VIEWINST", "1"));
	}

	m_Shader->AttachAndCompileShaderFromFile("Shadow\\depthbasic_vs_12", EShaderType::SHADER_VERTEX);
	if (LoadGeomShader)
	{
		if (!RHI::GetRenderSettings()->GetShadowSettings().UseViewInstancingForShadows && RHI::GetRenderSettings()->GetShadowSettings().UseGeometryShaderForShadows)
		{
			m_Shader->AttachAndCompileShaderFromFile("Shadow\\depthbasic_geo", EShaderType::SHADER_GEOMETRY);
		}
	}
	m_Shader->AttachAndCompileShaderFromFile("Shadow\\depthbasic_fs_12", EShaderType::SHADER_FRAGMENT);
	const int ShadowFarPlane = 500;
	zfar = static_cast<float>(ShadowFarPlane);
	if (RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS > 0)
	{
		ConstantBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant, Device);
		ConstantBuffer->CreateConstantBuffer(sizeof(LightData), RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS, true);
		GeometryProjections = RHI::CreateRHIBuffer(ERHIBufferType::Constant, Device);
		GeometryProjections->CreateConstantBuffer(sizeof(glm::mat4) * CUBE_SIDES, RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS, false);
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

std::vector<ShaderParameter> Shader_Depth::GetShaderParameters()
{
	std::vector<ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 0, Shader_Depth_RSSlots::ModelBuffer));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 1, Shader_Depth_RSSlots::GeometryProjections));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 2, Shader_Depth_RSSlots::VPBuffer));
	Output.push_back(ShaderParameter(ShaderParamType::RootConstant, 3, Shader_Depth_RSSlots::VI_Offset));
	return Output;
}

void Shader_Depth::UpdateGeometryShaderParams(glm::vec3 lightPos, glm::mat4 shadowProj, int index)
{
	glm::mat4 transforms[6];
	transforms[0] = (shadowProj *
		glm::lookAtRH(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));
	transforms[1] = (shadowProj *
		glm::lookAtRH(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));
	transforms[2] = (shadowProj *
		glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	transforms[3] = (shadowProj *
		glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	transforms[4] = (shadowProj *
		glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0)));
	transforms[5] = (shadowProj *
		glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0)));
	GeometryProjections->UpdateConstantBuffer(transforms, index);
}

void Shader_Depth::SetProjections(RHICommandList * list, int index)
{
	list->SetConstantBufferView(GeometryProjections, index, Shader_Depth_RSSlots::GeometryProjections);
}
