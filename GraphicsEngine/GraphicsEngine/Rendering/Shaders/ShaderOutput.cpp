#include "ShaderOutput.h"
#include "RHI/RHI.h"


ShaderOutput::ShaderOutput(int width, int height)
{
	m_Shader = RHI::CreateShaderProgam();

	m_Shader->CreateShaderProgram();
	if (RHI::GetType() == RenderSystemD3D12)
	{
		m_Shader->AttachAndCompileShaderFromFile("Pass_vs_12", SHADER_VERTEX);
		m_Shader->AttachAndCompileShaderFromFile("Pass_fs_12", SHADER_FRAGMENT);
	}
	else
	{

		m_Shader->AttachAndCompileShaderFromFile("Pass_vs", SHADER_VERTEX);
		m_Shader->AttachAndCompileShaderFromFile("Pass_fs", SHADER_FRAGMENT);
	}
	m_Shader->BindAttributeLocation(0, "pos");
	m_Shader->BindAttributeLocation(1, "Normal");
	m_Shader->BindAttributeLocation(2, "texCoords");


	m_Shader->BuildShaderProgram();
	m_Shader->ActivateShaderProgram();
	mwidth = width;
	mheight = height;

}

void ShaderOutput::Resize(int width, int height)
{
	mwidth = width;
	mheight = height;
}

ShaderOutput::~ShaderOutput()
{
}
void ShaderOutput::UpdateOGLUniforms(Transform *, Camera *, std::vector<Light*> lights)
{
}


void ShaderOutput::SetFullScreen(bool state)
{
	if (RHI::GetType() != RenderSystemOGL) 
	{
		return;
	}
	if (state)
	{
#if BUILD_OPENGL
		static const float g_quad_vertex_buffer_data[] = {
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f,  1.0f, 0.0f,
		};

		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
#endif
	}
	else
	{
		float h = 1.2f;
		float w = 1.2f;
		float xpos = -(w / 2.0f);
		float ypos = -(h / 2.0f) + 0.1f;
#if BUILD_OPENGL
		static const float g_quad_vertex_buffer_data[] = {
			xpos,     ypos + h,   0.0 ,
			xpos,     ypos,      0.0 ,
			xpos + w, ypos,       0.0 ,

			xpos,     ypos + h,   0.0 ,
			xpos + w, ypos,       0.0 ,
			xpos + w , ypos + h,  0.0 ,
		};
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
#endif
	}

}
std::vector<Shader::ShaderParameter> ShaderOutput::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> out;
	out.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	return out;
}
std::vector<Shader::VertexElementDESC> ShaderOutput::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}
void ShaderOutput::RenderPlane()
{

	if (RHI::GetType() == RenderSystemOGL)
	{
#if BUILD_OPENGL
		glUniform3fv(glGetUniformLocation(m_Shader->GetProgramHandle(), "R_inverseFilterTextureSize"), 1, glm::value_ptr(glm::vec3(1.0 / (float)mwidth, 1.0 / (float)mheight, 0.0)));
		glUniform1i(glGetUniformLocation(m_Shader->GetProgramHandle(), "EnableFXAA"), fxxastate ? 1 : 0);
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
#endif
	}
	else if (RHI::GetType() == RenderSystemD3D11)
	{
#if BUILD_D3D11
		OutputCube->Render();
#endif
	}
}

void ShaderOutput::SetFXAA(bool state)
{
	fxxastate = state;
}

void ShaderOutput::UpdateD3D11Uniforms(Transform *, Camera * c, std::vector<Light*> lights)
{
	m_cbuffer.m_worldMat = glm::mat4();
	m_cbuffer.m_viewMat = c->GetView();
	m_cbuffer.m_projection = c->GetProjection();
#if BUILD_D3D11
	RHI::GetD3DContext()->UpdateSubresource(RHI::instance->m_constantBuffer, 0, NULL, &m_cbuffer, 0, 0);
	RHI::GetD3DContext()->VSSetConstantBuffers(0, 1, &RHI::instance->m_constantBuffer);
#endif
}
