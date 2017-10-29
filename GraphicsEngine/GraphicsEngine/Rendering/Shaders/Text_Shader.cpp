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
}


Text_Shader::~Text_Shader()
{
}

void Text_Shader::UpdateOGLUniforms(Transform * t, Camera * c, std::vector<Light*> lights)
{
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(Width), 0.0f, static_cast<GLfloat>(Height));
	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgramHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3f(glGetUniformLocation(m_Shader->GetProgramHandle(), "textColor"), Colour.x, Colour.y, Colour.z);
	//glActiveTexture(GL_TEXTURE0);

}

void Text_Shader::UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights)
{
}
