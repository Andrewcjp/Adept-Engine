#include "Shader_Grass.h"
#include "RHI/RHI.h"


Shader_Grass::Shader_Grass()
{
	m_Shader->AttachAndCompileShaderFromFile("Grass", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Grass", EShaderType::SHADER_FRAGMENT);
}


Shader_Grass::~Shader_Grass()
{

}

//void Shader_Grass::UpdateOGLUniforms(Transform * t, Camera * c, std::vector<Light*> lights)
//{
//#if BUILD_OPENGL
//	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgramHandle(), "projMatrix"), 1, GL_FALSE, &c->GetProjection()[0][0]);
//	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgramHandle(), "viewMatrix"), 1, GL_FALSE, &c->GetView()[0][0]);
//	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgramHandle(), "modelMatrix"), 1, GL_FALSE, &t->GetModel()[0][0]);
//	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgramHandle(), "positionmat"), 1, GL_FALSE, &glm::translate(t->GetPos())[0][0]);
//	glUniform1f(glGetUniformLocation(m_Shader->GetProgramHandle(), "fAlphaTest"), 0.20f);
//
//	glUniform1f(glGetUniformLocation(m_Shader->GetProgramHandle(), "fAlphaMultiplier"), 1.1f);
//	glUniform1f(glGetUniformLocation(m_Shader->GetProgramHandle(), "fTimePassed"), 1.0f);
//	//glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "gSampler"), 0);
//	glUniform3f(glGetUniformLocation(m_Shader->GetProgramHandle(), "vColor"), 0.0f, 1.0f, 0.0f);
//	//glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgramHandle(), "projMatrix"), 1, GL_FALSE, &c->GetProjection()[0][0]);
//#endif
//}
