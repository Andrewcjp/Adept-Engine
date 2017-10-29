#include "Shader_Water.h"
#include "RHI/RHI.h"


//Unsued Shader

Shader_Water::Shader_Water()
{
	return;
	m_Shader = RHI::CreateShaderProgam();

	m_Shader->CreateShaderProgram();
	//m_Shader->AttachAndCompileShaderFromFile(L"../asset/shader/glsl/PBR.vert", SHADER_VERTEX);
	//m_Shader->AttachAndCompileShaderFromFile(L"../asset/shader/glsl/PBR.frag", SHADER_FRAGMENT);
	m_Shader->AttachAndCompileShaderFromFile("fallback", SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("fallback", SHADER_FRAGMENT);
	m_Shader->AttachAndCompileShaderFromFile("depthbasic", SHADER_GEOMETRY);//we can reuse the depth shaders geometry shader

	m_Shader->BindAttributeLocation(0, "pos");
	m_Shader->BindAttributeLocation(1, "Normal");
	m_Shader->BindAttributeLocation(2, "texCoords");

	glBindFragDataLocation(m_Shader->GetProgramHandle(), 0, "FragColor");

	m_Shader->BuildShaderProgram();
	m_Shader->ActivateShaderProgram();

	m_uniform_model = glGetUniformLocation(m_Shader->GetProgramHandle(), "model");
	m_uniform_View = glGetUniformLocation(m_Shader->GetProgramHandle(), "view");
	m_UniformMVP = glGetUniformLocation(m_Shader->GetProgramHandle(), "projection");

	Uniform_Cam_Pos = glGetUniformLocation(m_Shader->GetProgramHandle(), "camPos");
	m_uniform_texture = glGetUniformLocation(m_Shader->GetProgramHandle(), "albedoMap");
	m_uniform_LightNumber = glGetUniformLocation(m_Shader->GetProgramHandle(), "numLights");
	glUniform1i(m_uniform_texture, 0);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "NormalMap"), 4);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "metallicMap"), 2);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "roughnessMap"), 3);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "shadowcubemap"), 1);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "Displacementmap"), 5);
	m_IsMapUniform = glGetUniformLocation(m_Shader->GetProgramHandle(), "isMap");
	m_FarPlane = glGetUniformLocation(m_Shader->GetProgramHandle(), "far_plane");
	m_MV33 = glGetUniformLocation(m_Shader->GetProgramHandle(), "MV3x3");


	glGenTextures(1, (GLuint*)(&depthCubemap));

	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (GLuint i = 0; i < CUBE_SIDES; ++i) {//no more magic number
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, (GLuint*)(&m_reflectBuffer));
	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader

	glBindFramebuffer(GL_FRAMEBUFFER, m_reflectBuffer);
	//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthCubemap, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, depthCubemap, 0);

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("Water Buffer error\n");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Shader_Water::UpdateOGLUniforms(Transform * t, Camera * c, std::vector<Light*> lights)
{
	Light* light = lights[0];
	//for the geo shader
	GLfloat aspect = (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT;
	GLfloat znear = 1;
	GLfloat zfar = 10;
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, znear, zfar);
	glm::vec3 lightPos = light->GetPosition();
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


	for (GLuint i = 0; i < 6; ++i) {
		glUniformMatrix4fv(glGetUniformLocation(m_Shader->GetProgramHandle(), ("shadowMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(shadowTransforms[i]));
	}

	glUniformMatrix4fv(m_UniformMVP, 1, GL_FALSE, &c->GetProjection()[0][0]);
	glUniformMatrix4fv(m_uniform_View, 1, GL_FALSE, &c->GetView()[0][0]);
	glUniformMatrix4fv(m_uniform_model, 1, GL_FALSE, &t->GetModel()[0][0]);

	//	glm::mat3 mv33 = glm::mat3(t->GetModel() * c->GetView());
	//	glUniformMatrix3fv(m_MV33, 1, GL_FALSE, &mv33[0][0]);
	glUniform1i(m_uniform_LightNumber,static_cast<GLuint>(lights.size()));

	//glUniform3fv(Uniform_Cam_Pos, 1, &c->GetPosition()[0]);
	
	glUniform1f(m_FarPlane, 10);//find way to deal with this value
	glUniform1i(m_IsMapUniform, 0);

	for (int i = 0; i < lights.size(); i++) {
		glUniform3f(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].position").c_str()), lights[i]->GetPosition().x, lights[i]->GetPosition().y, lights[i]->GetPosition().z);
		glUniform3f(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].color").c_str()), lights[i]->GetColor().x, lights[i]->GetColor().y, lights[i]->GetColor().z);
	}
}

Shader_Water::~Shader_Water()
{
	glDeleteTextures(1, (GLuint*)(&depthCubemap));
}
void Shader_Water::SetShaderActive()
{
	m_Shader->ActivateShaderProgram();
	//bind the depth map to texture unit 1
	BindRefltBuffer();
}
void Shader_Water::BindRefltBuffer() {
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_reflectBuffer);
}
void Shader_Water::BindShadowmmap()
{
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
}

void Shader_Water::UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights)
{
}
