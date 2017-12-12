#include "Shader_Querry.h"
#include "RHI/RHI.h"


Shader_Querry::Shader_Querry()
{
	m_Shader = RHI::CreateShaderProgam();

	m_Shader->CreateShaderProgram();
	//m_Shader->AttachAndCompileShaderFromFile(L"../asset/shader/glsl/PBR.vert", SHADER_VERTEX);
	//m_Shader->AttachAndCompileShaderFromFile(L"../asset/shader/glsl/PBR.frag", SHADER_FRAGMENT);
	m_Shader->AttachAndCompileShaderFromFile("Querry", SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Querry", SHADER_FRAGMENT);

	m_Shader->BindAttributeLocation(0, "pos");

	glBindFragDataLocation(m_Shader->GetProgramHandle(), 0, "FragColor");

	m_Shader->BuildShaderProgram();
	m_Shader->ActivateShaderProgram();

	m_UniformMVP = glGetUniformLocation(m_Shader->GetProgramHandle(), "MVP");
}


Shader_Querry::~Shader_Querry()
{
}

void Shader_Querry::UpdateOGLUniforms(Transform * t, Camera * c, std::vector<Light*> lights)
{
	glUniformMatrix4fv(m_UniformMVP, 1, GL_FALSE, &t->GetMVP(c)[0][0]);
}

void Shader_Querry::UpdateD3D11Uniforms(Transform * , Camera * , std::vector<Light*> lights)
{
}
