#include "Shader_Depth.h"
#include "RHI/RHI.h"
#include "glm\glm.hpp"
#include "Shader_Main.h"


Shader_Depth::Shader_Depth(bool LoadGeo)
{
	LoadGeomShader = LoadGeo;
	m_Shader = RHI::CreateShaderProgam();
	m_Shader->CreateShaderProgram();
	if (!LoadGeomShader)
	{
		m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("DIRECTIONAL", "1"));
	}
	if (RHI::GetType() == RenderSystemD3D12)
	{
		m_Shader->AttachAndCompileShaderFromFile("depthbasic_vs_12", SHADER_VERTEX);
		if (LoadGeomShader)
		{
			m_Shader->AttachAndCompileShaderFromFile("depthbasic_geo", SHADER_GEOMETRY);
		}
		m_Shader->AttachAndCompileShaderFromFile("depthbasic_fs_12", SHADER_FRAGMENT);
	}

	m_Shader->BuildShaderProgram();
	m_Shader->ActivateShaderProgram();

	zfar = static_cast<float>(ShadowFarPlane);
	ConstantBuffer = RHI::CreateRHIBuffer(RHIBuffer::BufferType::Constant);
	ConstantBuffer->CreateConstantBuffer(sizeof(LightData), 1);
}
void Shader_Depth::UpdateBuffer(RHICommandList * list, LightData* data)
{
	ConstantBuffer->UpdateConstantBuffer(data, 0);
	list->SetConstantBufferView(ConstantBuffer, 0, 2);
}



Shader_Depth::~Shader_Depth()
{

}

void Shader_Depth::SetShaderActive()
{}

std::vector<Shader::ShaderParameter> Shader_Depth::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 0, 1));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 1, 0));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 2, 2));
	//Shader_Main::GetMainShaderSig(Output);
	return Output;
}

