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
}


Shader_Depth::~Shader_Depth()
{

}

void Shader_Depth::UpdateOGLUniforms(Transform * t, Camera *, std::vector<Light*> lights)
{
	//Light* light;
	//if (targetlight != nullptr)
	//{
	//	light = targetlight;
	//}
	//else
	//{
	//	light = lights[0];
	//}
	//glm::vec3 lightPos = light->GetPosition();
	//GLfloat aspect = (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT;
	//glm::mat4 shadowProj;
	//if (LoadGeomShader)
	//{
	//	shadowProj = glm::perspective(glm::radians(90.0f), aspect, znear, zfar);
	//}
	//else
	//{
	//	shadowProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, znear, zfar);
	//	//glm::mat4 lightview = glm::lookAt(light->GetPosition(), light->GetPosition() + light->GetDirection(), glm::vec3(0, 1, 0));//hmm not quite right
	//	glm::vec3 right = glm::cross(light->GetDirection(), glm::vec3(0, 1, 0));
	//	glm::vec3 up = glm::normalize(glm::cross(glm::normalize(right), light->GetDirection()));
	//	//	glm::mat4 lightview = glm::lookAt(lightPos, lightPos + light->GetDirection(), up);//hmm not quite right
	//	glm::mat4 lightview = glm::lookAt(lightPos, lightPos + light->GetDirection(), up);//hmm not quite right
	//	if (isnan(lightview[0][0]))
	//	{
	//		printf(" this NAN");
	//	}
	//	glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgramHandle(), "MVP"), 1, GL_FALSE, glm::value_ptr(shadowProj* lightview));
	//}


	//glUniformMatrix4fv(m_Model, 1, GL_FALSE, &t->GetModel()[0][0]);


	//glUniform1f(m_farplane, zfar);
	//glUniform3f(m_lightpos, light->GetPosition().x, light->GetPosition().y, light->GetPosition().z);

	//if (LoadGeomShader)
	//{
	//	std::vector<glm::mat4> shadowTransforms;
	//	shadowTransforms.push_back(shadowProj *
	//		glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	//	shadowTransforms.push_back(shadowProj *
	//		glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	//	shadowTransforms.push_back(shadowProj *
	//		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	//	shadowTransforms.push_back(shadowProj *
	//		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	//	shadowTransforms.push_back(shadowProj *
	//		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
	//	shadowTransforms.push_back(shadowProj *
	//		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));


	//	for (GLuint i = 0; i < 6; ++i)
	//	{
	//		glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgramHandle(), ("shadowMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(shadowTransforms[i]));
	//	}
	//}
	////glUniformMatrix4fv(m_DepthMVP, 1, GL_FALSE, &depthMVP[0][0]);

}

void Shader_Depth::SetShaderActive()
{}

std::vector<Shader::ShaderParameter> Shader_Depth::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;

	//Output.resize(3);
	//Output[0] = ShaderParameter(ShaderParamType::CBV, 0, 0);
	//Output[1] = ShaderParameter(ShaderParamType::CBV, 1, 1);
	//Output[2] = ShaderParameter(ShaderParamType::CBV, 2, 2);
	Shader_Main::GetMainShaderSig(Output);
	return Output;
}
