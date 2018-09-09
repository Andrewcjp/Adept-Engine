#include "Shader_Particle.h"
#include "RHI/RHI.h"


Shader_Particle::Shader_Particle()
{
	m_Shader = RHI::CreateShaderProgam();
	
	//m_Shader->AttachAndCompileShaderFromFile(L"asset/shader/glsl/PBR.vert", EShaderType::SHADER_VERTEX);
	//m_Shader->AttachAndCompileShaderFromFile(L"asset/shader/glsl/PBR.frag", EShaderType::SHADER_FRAGMENT);
	m_Shader->AttachAndCompileShaderFromFile("Particle", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Particle", EShaderType::SHADER_FRAGMENT);

#if BUILD_OPENGL
	VP = glGetUniformLocation(m_Shader->GetProgramHandle(), "VP");
	CameraRight_worldspace = glGetUniformLocation(m_Shader->GetProgramHandle(), "CameraRight_worldspace");
	CameraUp_worldspace = glGetUniformLocation(m_Shader->GetProgramHandle(), "CameraUp_worldspace");
	//glGetUniformLocation(m_Shader->GetProgramHandle(), "CameraUp_worldspace");
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "Texture"), 0);
#endif

}


Shader_Particle::~Shader_Particle()
{
}
void Shader_Particle::UpdateOGLUniforms(Transform* , Camera* c, std::vector<Light*> lights)
{
//	glUniformMatrix4fv(VP, 1, GL_FALSE, &c->GetViewProjection()[0][0]);
//	glUniform3fv(CameraRight_worldspace, 1, glm::value_ptr(c->GetRight()));
//	glUniform3fv(CameraUp_worldspace, 1, glm::value_ptr(c->GetUp()));

}

void Shader_Particle::UpdateD3D11Uniforms(Transform * , Camera * , std::vector<Light*> lights)
{
}
