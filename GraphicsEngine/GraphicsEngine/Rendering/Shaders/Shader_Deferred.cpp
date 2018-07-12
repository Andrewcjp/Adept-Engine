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

	//Initialise OGL shader
	m_Shader = RHI::CreateShaderProgam();

	m_Shader->CreateShaderProgram();
	m_Shader->AttachAndCompileShaderFromFile("Deferred_LightingPass_vs", SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Deferred_LightingPass_fs", SHADER_FRAGMENT);

	m_Shader->BuildShaderProgram();
	m_Shader->ActivateShaderProgram();

}


Shader_Deferred::~Shader_Deferred()
{
}
std::vector<Shader::ShaderParameter> Shader_Deferred::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> out;
	out.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	out.push_back(ShaderParameter(ShaderParamType::SRV, 1, 1));
	out.push_back(ShaderParameter(ShaderParamType::CBV, 2, 1));
	out.push_back(ShaderParameter(ShaderParamType::SRV, 3, 2));
	out.push_back(ShaderParameter(ShaderParamType::CBV, 4, 2));
	out.push_back(ShaderParameter(ShaderParamType::SRV, 5, 10));
	out.push_back(ShaderParameter(ShaderParamType::SRV, 6, 11));
	out.push_back(ShaderParameter(ShaderParamType::SRV, 7, 12));
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