#include "stdafx.h"
#include "Text_Shader.h"
#include "RHI/RHI.h"
#include "glm\glm.hpp"
Text_Shader::Text_Shader()
{
	m_Shader = RHI::CreateShaderProgam();
	m_Shader->CreateShaderProgram();
	m_Shader->AttachAndCompileShaderFromFile("text_vs_Atlas", SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("text_fs_Atlas", SHADER_FRAGMENT);

	m_Shader->BindAttributeLocation(0, "vertex");

	m_Shader->BuildShaderProgram();
	CBV = RHI::CreateRHIBuffer(RHIBuffer::BufferType::Constant);
	CBV->CreateConstantBuffer(sizeof(Data), 1);
} 

std::vector<Shader::ShaderParameter> Text_Shader::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> out;
	out.resize(2);
	out[0] = ShaderParameter(ShaderParamType::SRV, 0, 0);
	out[1] = ShaderParameter(ShaderParamType::CBV, 1, 0);
	return out;
}
std::vector<Shader::VertexElementDESC> Text_Shader::GetVertexFormat()
{
	std::vector<Shader::VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	//out.push_back(VertexElementDESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}
void Text_Shader::Update(RHICommandList* lsit)
{
	Data.proj = glm::ortho(0.0f, (float)(Width), 0.0f, (float)(Height));
	Data.Colour = Colour;
	CBV->UpdateConstantBuffer(&Data, 0);
	lsit->SetConstantBufferView(CBV, 0, 1);
}


Text_Shader::~Text_Shader()
{
}

void Text_Shader::UpdateOGLUniforms(Transform * t, Camera * c, std::vector<Light*> lights)
{
	UNUSED_PARAM(c);
	UNUSED_PARAM(t);

	//glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(Width), 0.0f, static_cast<GLfloat>(Height));
	//glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgramHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//glUniform3f(glGetUniformLocation(m_Shader->GetProgramHandle(), "textColor"), Colour.x, Colour.y, Colour.z);
	////glActiveTexture(GL_TEXTURE0);

}

void Text_Shader::UpdateD3D11Uniforms(Transform * , Camera * , std::vector<Light*> lights)
{
}
