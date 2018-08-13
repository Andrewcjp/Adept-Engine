#include "Shader_Deferred.h"
#include "RHI/ShaderProgramBase.h"


Shader_Deferred::Shader_Deferred()
{
	float g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,0.0f,
		1.0f, -1.0f, 0.0f,0.0f,
		-1.0f,  1.0f, 0.0f,0.0f,
		-1.0f,  1.0f, 0.0f,0.0f,
		1.0f, -1.0f, 0.0f,0.0f,
		1.0f,  1.0f, 0.0f,0.0f,
	};
	VertexBuffer = RHI::CreateRHIBuffer(RHIBuffer::BufferType::Vertex);
	VertexBuffer->CreateVertexBuffer(sizeof(float) * 4, sizeof(float) * 6 * 4);
	VertexBuffer->UpdateVertexBuffer(&g_quad_vertex_buffer_data, sizeof(float) * 6 * 4);

	//Initialise shader
	m_Shader = RHI::CreateShaderProgam();

	m_Shader->CreateShaderProgram();
	m_Shader->AttachAndCompileShaderFromFile("Deferred_LightingPass_vs", SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Deferred_LightingPass_fs", SHADER_FRAGMENT);

	m_Shader->BuildShaderProgram();
	m_Shader->ActivateShaderProgram();

}

Shader_Deferred::~Shader_Deferred()
{
	delete VertexBuffer;
}

std::vector<Shader::ShaderParameter> Shader_Deferred::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> out;
	out.push_back(ShaderParameter(ShaderParamType::CBV, DeferredLightingShaderRSBinds::LightDataCBV, 1));
	out.push_back(ShaderParameter(ShaderParamType::CBV, DeferredLightingShaderRSBinds::MVCBV, 2));

	out.push_back(ShaderParameter(ShaderParamType::SRV, DeferredLightingShaderRSBinds::PosTex, 0));
	out.push_back(ShaderParameter(ShaderParamType::SRV, DeferredLightingShaderRSBinds::NormalTex, 1));
	out.push_back(ShaderParameter(ShaderParamType::SRV, DeferredLightingShaderRSBinds::AlbedoTex, 2));

	out.push_back(ShaderParameter(ShaderParamType::SRV, DeferredLightingShaderRSBinds::DirShadow, 7));
	out.push_back(ShaderParameter(ShaderParamType::SRV, DeferredLightingShaderRSBinds::PointShadow, 8));


	out.push_back(ShaderParameter(ShaderParamType::SRV, DeferredLightingShaderRSBinds::DiffuseIr, 10));
	out.push_back(ShaderParameter(ShaderParamType::SRV, DeferredLightingShaderRSBinds::SpecBlurMap, 11));
	out.push_back(ShaderParameter(ShaderParamType::SRV, DeferredLightingShaderRSBinds::EnvBRDF, 12));
	//out.push_back(ShaderParameter(ShaderParamType::SRV, 8, 13));
	return out;
}
std::vector<Shader::VertexElementDESC> Shader_Deferred::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}

void Shader_Deferred::RenderScreenQuad(RHICommandList * list)
{
	//todo: less than full screen!
	list->SetVertexBuffer(VertexBuffer);
	list->DrawPrimitive(6, 1, 0, 0);

}