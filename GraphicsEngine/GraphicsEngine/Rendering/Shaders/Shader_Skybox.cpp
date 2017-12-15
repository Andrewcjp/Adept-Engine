#include "Shader_Skybox.h"
#include "RHI/RHI.h"
#include "Core/Assets/ImageLoader.h"

Shader_Skybox::Shader_Skybox()
{
	if (RHI::GetType() == RenderSystemOGL)
	{
		m_Shader = RHI::CreateShaderProgam();
		m_Shader->CreateShaderProgram();
		m_Shader->AttachAndCompileShaderFromFile("skybox", SHADER_VERTEX);
		m_Shader->AttachAndCompileShaderFromFile("skybox", SHADER_FRAGMENT);

		m_Shader->BindAttributeLocation(0, "position");

		m_Shader->BuildShaderProgram();
		m_Shader->ActivateShaderProgram();
		SkyboxTexture = ImageLoader::instance->loadsplitCubeMap("heh");
	}
}


Shader_Skybox::~Shader_Skybox()
{
	glDeleteTextures(1, &SkyboxTexture);
}

void Shader_Skybox::UpdateOGLUniforms(Transform * , Camera * c, std::vector<Light*> lights)
{
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, SkyboxTexture);

	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "skybox"), 2);
	glDepthMask(GL_FALSE);
	glm::mat4 view;

	view = glm::mat4(glm::mat3(c->GetView()));
	

	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgramHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(c->GetProjection()));
	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgramHandle(), "view"), 1, GL_FALSE, glm::value_ptr(view));
	glDepthFunc(GL_LEQUAL);
}

void Shader_Skybox::UpdateD3D11Uniforms(Transform * , Camera * , std::vector<Light*> lights)
{
}
