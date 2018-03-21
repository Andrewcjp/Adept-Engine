#include "Shader_Depth.h"
#include "RHI/RHI.h"
#include "glm\glm.hpp"
#include "Shader_Main.h"
void Shader_Depth::INIT()
{
	m_Shader = RHI::CreateShaderProgam();
	m_Shader->CreateShaderProgram();
	if (RHI::GetType() == RenderSystemD3D12)
	{
		m_Shader->AttachAndCompileShaderFromFile("depthbasic_vs_12", SHADER_VERTEX);
		m_Shader->AttachAndCompileShaderFromFile("depthbasic_fs_12", SHADER_FRAGMENT);
	}
	else
	{
		m_Shader->AttachAndCompileShaderFromFile("depthbasic_vs", SHADER_VERTEX);
		m_Shader->AttachAndCompileShaderFromFile("depthbasic_fs", SHADER_FRAGMENT);
	}

	if (LoadGeomShader)
	{
		m_Shader->AttachAndCompileShaderFromFile("depthbasic_geo", SHADER_GEOMETRY);
		//		std::cout << LoadGeomShader << " is wrong" << std::endl;
	}

	m_Shader->BindAttributeLocation(0, "position");

	m_Shader->BuildShaderProgram();
	m_Shader->ActivateShaderProgram();
	if (RHI::GetType() == RenderSystemOGL)
	{
		m_Model = glGetUniformLocation(m_Shader->GetProgramHandle(), "model");
		m_farplane = glGetUniformLocation(m_Shader->GetProgramHandle(), "far_plane");
		m_lightpos = glGetUniformLocation(m_Shader->GetProgramHandle(), "lightPos");
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "Ispoint"), LoadGeomShader);
	}

	if (LoadGeomShader)
	{

		shadowbuffer = RHI::CreateFrameBuffer(SHADOW_WIDTH, SHADOW_HEIGHT, 1, FrameBuffer::CubeDepth);
	}
	else
	{
		shadowbuffer = RHI::CreateFrameBuffer(SHADOW_WIDTH, SHADOW_HEIGHT, 1, FrameBuffer::Depth);
	}
	zfar = static_cast<GLfloat>(ShadowFarPlane);
	shadowbuffer->UnBind();
}

Shader_Depth::Shader_Depth(Light* l, bool LoadGeo)
{
	LoadGeomShader = LoadGeo;
	targetlight = l;
	INIT();
}


Shader_Depth::~Shader_Depth()
{
	delete shadowbuffer;
	delete CubeMaptex;
}

void Shader_Depth::UpdateOGLUniforms(Transform * t, Camera * , std::vector<Light*> lights)
{
	Light* light;
	if (targetlight != nullptr)
	{
		light = targetlight;
	}
	else
	{
		light = lights[0];
	}
	glm::vec3 lightPos = light->GetPosition();
	GLfloat aspect = (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT;
	glm::mat4 shadowProj;
	if (LoadGeomShader)
	{
		shadowProj = glm::perspective(glm::radians(90.0f), aspect, znear, zfar);
	}
	else
	{
		shadowProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, znear, zfar);
		//glm::mat4 lightview = glm::lookAt(light->GetPosition(), light->GetPosition() + light->GetDirection(), glm::vec3(0, 1, 0));//hmm not quite right
		glm::vec3 right = glm::cross(light->GetDirection(), glm::vec3(0, 1, 0));
		glm::vec3 up = glm::normalize(glm::cross(glm::normalize(right), light->GetDirection()));
		//	glm::mat4 lightview = glm::lookAt(lightPos, lightPos + light->GetDirection(), up);//hmm not quite right
		glm::mat4 lightview = glm::lookAt(lightPos, lightPos + light->GetDirection(), up);//hmm not quite right
		if (isnan(lightview[0][0]))
		{
			printf(" this NAN");
		}
		glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgramHandle(), "MVP"), 1, GL_FALSE, glm::value_ptr(shadowProj* lightview));
	}


	glUniformMatrix4fv(m_Model, 1, GL_FALSE, &t->GetModel()[0][0]);


	glUniform1f(m_farplane, zfar);
	glUniform3f(m_lightpos, light->GetPosition().x, light->GetPosition().y, light->GetPosition().z);

	if (LoadGeomShader)
	{
		std::vector<glm::mat4> shadowTransforms;
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));


		for (GLuint i = 0; i < 6; ++i)
		{
			glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgramHandle(), ("shadowMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(shadowTransforms[i]));
		}
	}
	//glUniformMatrix4fv(m_DepthMVP, 1, GL_FALSE, &depthMVP[0][0]);

}

void Shader_Depth::SetShaderActive()
{
}

void Shader_Depth::SetShaderActive(CommandListDef* List)
{
	m_Shader->ActivateShaderProgram();

	shadowbuffer->BindBufferAsRenderTarget(List);
	shadowbuffer->ClearBuffer(List);
}

void Shader_Depth::BindShadowmmap(CommandListDef* List)
{
	if (LoadGeomShader == false)
	{
		if (RHI::GetType() != RenderSystemD3D12)
		{
			shadowbuffer->BindToTextureUnit(SHADOWDIRMAP1);
		}
		else
		{
			//((D3D12FrameBuffer*)shadowbuffer)->BindBufferToTexture(List,0);
		}
		//printf("bind");
	}
	else
	{
		if (targetlight != nullptr)
		{
			shadowbuffer->BindToTextureUnit(SHADOWCUBEMAP + targetlight->GetShadowId());
		}
		else
		{
			shadowbuffer->BindToTextureUnit(SHADOWCUBEMAP);
		}
		//	shadowbuffer->BindToTextureUnit(10);
	}
	//	glActiveTexture(GL_TEXTURE1);
	//	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowbuffer->GetRenderTexture()->m_syshandle);
}
std::vector<Shader::ShaderParameter> Shader_Depth::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Shader_Main::GetMainShaderSig(Output);
	return Output;
}
void Shader_Depth::UpdateD3D11Uniforms(Transform * t, Camera * , std::vector<Light*> lights)
{
#if BUILD_D3D11
	glm::mat4 shadowProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, znear, zfar);

	glm::vec3 at = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 lightview = glm::lookAtLH(lights[0]->GetPosition(), at, up);

	m_cbuffer.m_projection = shadowProj;
	m_cbuffer.m_viewMat = lightview;
	m_cbuffer.m_worldMat = t->GetModel();
	RHI::GetD3DContext()->UpdateSubresource(RHI::instance->m_constantBuffer, 0, NULL, &m_cbuffer, 0, 0);
	RHI::GetD3DContext()->VSSetConstantBuffers(0, 1, &RHI::instance->m_constantBuffer);
#endif
}
