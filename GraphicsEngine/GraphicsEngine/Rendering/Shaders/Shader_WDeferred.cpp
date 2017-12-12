#include "Shader_WDeferred.h"
#include "RHI/RHI.h"


Shader_WDeferred::Shader_WDeferred()
{
	//Initialise OGL shader
	m_Shader = RHI::CreateShaderProgam();

	m_Shader->CreateShaderProgram();
	m_Shader->AttachAndCompileShaderFromFile("DeferredWrite", SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("DeferredWrite", SHADER_FRAGMENT);

	m_Shader->BindAttributeLocation(0, "pos");
	m_Shader->BindAttributeLocation(1, "Normal");
	m_Shader->BindAttributeLocation(2, "texCoords");

	//	glBindFragDataLocation(m_Shader->GetProgramHandle(), 0, "FragColor");

	m_Shader->BuildShaderProgram();
	m_Shader->ActivateShaderProgram();
	m_uniform_model = glGetUniformLocation(m_Shader->GetProgramHandle(), "model");
	m_uniform_View = glGetUniformLocation(m_Shader->GetProgramHandle(), "view");
	m_UniformMVP = glGetUniformLocation(m_Shader->GetProgramHandle(), "projection");

	Uniform_Cam_Pos = glGetUniformLocation(m_Shader->GetProgramHandle(), "campos");
	m_uniform_texture = glGetUniformLocation(m_Shader->GetProgramHandle(), "albedoMap");
	m_uniform_LightNumber = glGetUniformLocation(m_Shader->GetProgramHandle(), "numLights");
	glUniform1i(m_uniform_texture, ALBEDOMAP);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "NormalMap"), NORMALMAP);

	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "shadowdirmap"), SHADOWDIRMAP1);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "Displacementmap"), DISPMAP);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "Reflectmap"), 10);
	m_IsMapUniform = glGetUniformLocation(m_Shader->GetProgramHandle(), "isMap");
	m_FarPlane = glGetUniformLocation(m_Shader->GetProgramHandle(), "far_plane");
	m_MV33 = glGetUniformLocation(m_Shader->GetProgramHandle(), "MV3x3");
	IsReflect = glGetUniformLocation(m_Shader->GetProgramHandle(), "IsReft");
}


Shader_WDeferred::~Shader_WDeferred()
{
}

void Shader_WDeferred::SetNormalState(bool hasnormalmap)
{

	if (hasnormalmap)
	{
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "HasNormalMap"), 1);
	}
	else
	{
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "HasNormalMap"), 0);
	}

}

void Shader_WDeferred::UpdateOGLUniforms(Transform * t, Camera * c, std::vector<Light*> lights)
{
	glUniformMatrix4fv(m_UniformMVP, 1, GL_FALSE, &c->GetProjection()[0][0]);
	glUniformMatrix4fv(m_uniform_View, 1, GL_FALSE, &c->GetView()[0][0]);
	glUniformMatrix4fv(m_uniform_model, 1, GL_FALSE, &t->GetModel()[0][0]);

	glUniform1i(m_uniform_LightNumber, static_cast<GLuint>(lights.size()));

	glUniform1f(m_FarPlane, static_cast<GLfloat>(ShadowFarPlane));
	glUniform1i(m_IsMapUniform, 0);
	glUniform3fv(Uniform_Cam_Pos, 1, glm::value_ptr(c->GetPosition()));
}

void Shader_WDeferred::UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights)
{
	UNUSED_PARAM(t);
	UNUSED_PARAM(c);
}
