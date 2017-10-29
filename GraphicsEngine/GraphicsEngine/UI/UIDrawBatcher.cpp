#include "stdafx.h"
#include "UIDrawBatcher.h"
#include "UIManager.h"
UIDrawBatcher* UIDrawBatcher::instance = nullptr;
UIDrawBatcher::UIDrawBatcher()
{
	instance = this;
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	m_TextShader = new OGLShaderProgram();
	m_TextShader->CreateShaderProgram();
	m_TextShader->AttachAndCompileShaderFromFile("UI_Batch_vs", SHADER_VERTEX);
	m_TextShader->AttachAndCompileShaderFromFile("UI_Batch_fs", SHADER_FRAGMENT);

	m_TextShader->BindAttributeLocation(0, "vertex");

	m_TextShader->BuildShaderProgram();
	m_TextShader->ActivateShaderProgram();
}


UIDrawBatcher::~UIDrawBatcher()
{
}
void UIDrawBatcher::SendToGPU()
{
	if (BatchedVerts.size() == 0)
	{
		return;
	}
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(UIVertex)*BatchedVerts.size(), &BatchedVerts[0], GL_DYNAMIC_DRAW);
}
void UIDrawBatcher::RenderBatches()
{
	if (BatchedVerts.size() == 0)
	{
		return;
	}
	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 0);
	glVertexAttribDivisor(2, 0);
	m_TextShader->ActivateShaderProgram();
	glDisable(GL_BLEND);	
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(UIManager::instance->GetWidth()), 0.0f, static_cast<GLfloat>(UIManager::instance->GetHeight()));
	glUniformMatrix4fv(glGetUniformLocation(m_TextShader->GetProgramHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	//glUniform1i(glGetUniformLocation(m_TextShader->GetProgramHandle(), "UseColour"), (texture == nullptr));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	GLsizei size = (sizeof(UIVertex));
	glVertexAttribDivisor(1, 0);//particles did not clear 
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		1,                  // size
		GL_INT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)(sizeof(GLfloat) * 2)            // array buffer offset
	);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)((sizeof(GLfloat) * 2) + sizeof(GLint))            // array buffer offset
	);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(
		3,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)(0 + sizeof(glm::vec2) + sizeof(GLint) + sizeof(glm::vec3))          // array buffer offset
	);	
	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)BatchedVerts.size()); // 2*3 indices starting at 0 -> 2 triangles

	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);	
}
void UIDrawBatcher::AddVertex(glm::vec2 pos, bool Back, glm::vec3 frontcol, glm::vec3 backcol)
{
	UIVertex vert;
	vert.position = pos;
	vert.UseBackgound = Back;
	vert.FrontColour = frontcol;
	vert.BackColour = backcol;
	BatchedVerts.push_back(vert);
}


void UIDrawBatcher::ClearVertArray()
{
	BatchedVerts.clear();
}
