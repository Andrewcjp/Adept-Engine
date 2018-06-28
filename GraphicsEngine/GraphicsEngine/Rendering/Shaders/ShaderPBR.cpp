#include "ShaderPBR.h"
#include "RHI/RHI.h"


ShaderPBR::ShaderPBR()
{

	//Initialise OGL shader
	m_Shader = RHI::CreateShaderProgam();

	m_Shader->CreateShaderProgram();
	m_Shader->AttachAndCompileShaderFromFile("PBR", SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("PBR", SHADER_FRAGMENT);
	/*m_Shader->AttachAndCompileShaderFromFile(L"asset/shader/glsl/fallback.vert", SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile(L"asset/shader/glsl/fallback.frag", SHADER_FRAGMENT);*/

	m_Shader->BindAttributeLocation(0, "pos");
	m_Shader->BindAttributeLocation(1, "Normal");
	m_Shader->BindAttributeLocation(2, "texCoords");
#if BUILD_OPENGL
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
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "NormalMap"), NORMALMAP);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "metallicMap"), 2);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "roughnessMap"), 3);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "shadowcubemap"), SHADOWCUBEMAP);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "Displacementmap"), DISPMAP);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "Reflectmap"), 10);
	m_IsMapUniform = glGetUniformLocation(m_Shader->GetProgramHandle(), "isMap");
	m_FarPlane = glGetUniformLocation(m_Shader->GetProgramHandle(), "far_plane");
	m_MV33 = glGetUniformLocation(m_Shader->GetProgramHandle(), "MV3x3");
	IsReflect = glGetUniformLocation(m_Shader->GetProgramHandle(), "IsReft");
#endif
	
}



ShaderPBR::~ShaderPBR()
{
}

void ShaderPBR::UpdateOGLUniforms(Transform* t, Camera* c, std::vector<Light*> lights)
{

//	glUniformMatrix4fv(m_UniformMVP, 1, GL_FALSE, &c->GetProjection()[0][0]);
//	glUniformMatrix4fv(m_uniform_View, 1, GL_FALSE, &c->GetView()[0][0]);
//	glUniformMatrix4fv(m_uniform_model, 1, GL_FALSE, &t->GetModel()[0][0]);

	//	glm::mat3 mv33 = glm::mat3(t->GetModel() * c->GetView());
	//	glUniformMatrix3fv(m_MV33, 1, GL_FALSE, &mv33[0][0]);


	//glUniform3fv(Uniform_Cam_Pos, 1, &c->GetPosition()[0]);

//	glUniform1f(m_FarPlane, static_cast<float>(ShadowFarPlane));//find way to deal with this value
//	glUniform1i(m_IsMapUniform, 0);
#if BUILD_OPENGL
	for (int i = 0; i < lights.size(); i++) {
		glUniform3f(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].position").c_str()), lights[i]->GetPosition().x, lights[i]->GetPosition().y, lights[i]->GetPosition().z);
		glUniform3f(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].color").c_str()), lights[i]->GetColor().x, lights[i]->GetColor().y, lights[i]->GetColor().z);
	}
#endif
}

void ShaderPBR::UpdateD3D11Uniforms(Transform * , Camera * , std::vector<Light*> lights)
{
}
