#include "Shader_Deferred.h"
#include "OpenGL/OGLShaderProgram.h"


Shader_Deferred::Shader_Deferred()
{
	//Initialise OGL shader
	m_Shader = new OGLShaderProgram();

	m_Shader->CreateShaderProgram();
	m_Shader->AttachAndCompileShaderFromFile("Deferred", SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Deferred", SHADER_FRAGMENT);

	m_Shader->BindAttributeLocation(0, "pos");
	m_Shader->BindAttributeLocation(1, "Normal");
	m_Shader->BindAttributeLocation(2, "texCoords");

	//glBindFragDataLocation(m_Shader->GetProgramHandle(), 0, "FragColor");

	m_Shader->BuildShaderProgram();
	m_Shader->ActivateShaderProgram();
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "gPosition"), 0);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "gNormal"), 1);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "gAlbedoSpec"), 2);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "gTangent"), 3);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "gDisplacement"), 4);
	//
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "shadowcubemap"), SHADOWCUBEMAP);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "shadowcubemap2"), SHADOWCUBEMAP2);
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "ssao"), 6);

	static const GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
	};


	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
}


Shader_Deferred::~Shader_Deferred()
{
}

void Shader_Deferred::UpdateOGLUniforms(Transform * t, Camera * c, std::vector<Light*> lights)
{
	glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "numLights"), static_cast<GLuint>(lights.size()));
	glUniform1f(glGetUniformLocation(m_Shader->GetProgramHandle(), "far_plane"), static_cast<GLfloat>(ShadowFarPlane));
	glUniform3fv(glGetUniformLocation(m_Shader->GetProgramHandle(), "viewPos"), 1, &c->GetPosition()[0]);
	for (int i = 0; i < lights.size(); i++) {
		glUniform3f(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].position").c_str()), lights[i]->GetPosition().x, lights[i]->GetPosition().y, lights[i]->GetPosition().z);
		glUniform3f(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].Color").c_str()), lights[i]->GetColor().x, lights[i]->GetColor().y, lights[i]->GetColor().z);
		glUniform3fv(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].Direction").c_str()), 1, glm::value_ptr(lights[i]->GetDirection()));
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].type").c_str()), static_cast<int>(lights[i]->GetType()));
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].ShadowID").c_str()), (lights[i]->GetShadowId()));
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].DirShadowID").c_str()), (lights[i]->DirectionalShadowid));
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), ("allLights[" + std::to_string(i) + "].HasShadow").c_str()), (lights[i]->GetDoesShadow()));
	}
}
void Shader_Deferred::RenderPlane()
{
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

	glDisableVertexAttribArray(0);
}

void Shader_Deferred::UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights)
{
}
