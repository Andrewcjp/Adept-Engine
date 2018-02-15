#include "stdafx.h"
#include "LineDrawer.h"
#include "OpenGL\OGLShaderProgram.h"
#include "RHI/RHI.h"
#include "glm\glm.hpp"
#include "UI\UIManager.h"
LineDrawer::LineDrawer()
{
	if (RHI::IsOpenGL())
	{
		InitOGL();
	}

	//todo: memeory
	CurrentAllocateLines = 75;
	Lines = new Line[CurrentAllocateLines];
	Verts = new float[CurrentAllocateLines * 10];
}
void LineDrawer::InitOGL()
{
	m_TextShader = new OGLShaderProgram();
	m_TextShader->CreateShaderProgram();
	m_TextShader->AttachAndCompileShaderFromFile("line_vs", SHADER_VERTEX);
	m_TextShader->AttachAndCompileShaderFromFile("line_fs", SHADER_FRAGMENT);
	m_TextShader->BindAttributeLocation(0, "vertex");
	m_TextShader->BuildShaderProgram();
	m_TextShader->ActivateShaderProgram();
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * CurrentAllocateLines * 10, NULL, GL_STREAM_DRAW);
}

LineDrawer::~LineDrawer()
{
	delete Lines;
	delete Verts;
	if (RHI::IsOpenGL())
	{
		glDeleteBuffers(1, &quad_vertexbuffer);
	}
}

void LineDrawer::GenerateLines()
{
	if (LineCount == 0)
	{
		return;
	}
	vertindex = 0;
	for (int i = 0; i < LineCount; i++)
	{
		Verts[vertindex] = (Lines[i].startpos.x);
		Verts[vertindex + 1] = (Lines[i].startpos.y);
		Verts[vertindex + 2] = (Lines[i].colour.x);
		Verts[vertindex + 3] = (Lines[i].colour.y);
		Verts[vertindex + 4] = (Lines[i].colour.z);
		vertindex += 5;
		Verts[vertindex] = (Lines[i].endpos.x);
		Verts[vertindex + 1] = (Lines[i].endpos.y);
		Verts[vertindex + 2] = (Lines[i].colour.x);
		Verts[vertindex + 3] = (Lines[i].colour.y);
		Verts[vertindex + 4] = (Lines[i].colour.z);
		vertindex += 5;
	}
	VertsOnGPU = vertindex;

}//use one array?
void LineDrawer::RenderLines()
{
	if (RHI::IsOpenGL())
	{
		RenderLines_OpenGL();
	}
	//bin all lines rendered this frame.
	ClearLines();
}
void LineDrawer::RenderLines_OpenGL()
{
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertindex, NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * vertindex, &Verts[0]);
	if (VertsOnGPU != 0)
	{
		glDisable(GL_BLEND);
		m_TextShader->ActivateShaderProgram();

		glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(UIManager::instance->GetWidth()), 0.0f, static_cast<GLfloat>(UIManager::instance->GetHeight()));
		glUniformMatrix4fv(glGetUniformLocation(m_TextShader->GetProgramHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		//glUniform3f(glGetUniformLocation(m_TextShader->GetProgramHandle(), "textColor"), Colour.x, Colour.y, Colour.z);

		GLsizei size = (5 * sizeof(GLfloat));
		//	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			2,                  // size 2items
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			size,                  // stride
			(void*)0            // array buffer offset
		);
		glEnableVertexAttribArray(0);
		glVertexAttribDivisor(1, 0);//particles did not clear 
		glVertexAttribPointer(
			1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size 
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			size,                  // stride
			(void*)(2 * sizeof(GLfloat))            // array buffer offset
		);

		glEnableVertexAttribArray(1);
		// Draw the triangles !
		glDrawArrays(GL_LINES, 0, (GLsizei)(LineCount * 2)); // 2*3 indices starting at 0 -> 2 triangles
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);
	}
}
void LineDrawer::ClearLines()
{
	LineCount = 0;
}

void LineDrawer::AddLine(glm::vec2 Start, glm::vec2 end, glm::vec3 colour, float)
{
	Line l;
	l.startpos = Start;
	l.endpos = end;
	l.colour = colour;
	Lines[LineCount] = l;
	LineCount++;
}
