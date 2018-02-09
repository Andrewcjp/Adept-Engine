#include "stdafx.h"
#include "DebugLineDrawer.h"
#include "OpenGL/OGLShaderProgram.h"
#include "RHI/RHI.h"
#include "glm\glm.hpp"
#include "UI\UIManager.h"
#include "../Editor/EditorWindow.h"
DebugLineDrawer* DebugLineDrawer::instance = nullptr;
DebugLineDrawer::DebugLineDrawer()
{
	if (RHI::GetType() == RenderSystemOGL)
	{
		m_TextShader = new OGLShaderProgram();
		m_TextShader->CreateShaderProgram();
		m_TextShader->AttachAndCompileShaderFromFile("Debugline_vs", SHADER_VERTEX);
		m_TextShader->AttachAndCompileShaderFromFile("Debugline_fs", SHADER_FRAGMENT);

		m_TextShader->BindAttributeLocation(0, "vertex");

		m_TextShader->BuildShaderProgram();
		m_TextShader->ActivateShaderProgram();

		//	texture = RHI::CreateTexture("../asset/texture/UI/window.png");
		glGenBuffers(1, &quad_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		instance = this;
	}
}


DebugLineDrawer::~DebugLineDrawer()
{
	glDeleteBuffers(1, &quad_vertexbuffer);
}

void DebugLineDrawer::GenerateLines()
{
	if (Lines.size() == 0)
	{
		return;
	}
	std::vector<GLfloat> Verts;
	for (int i = 0; i < Lines.size(); i++)
	{
		/*GLfloat w = Lines[i].Thickness;
		GLfloat h = Lines[i].Thickness;*/
		//Lines[i].colour = glm::vec3(1, 1, 0);
		Verts.push_back(Lines[i].startpos.x);
		Verts.push_back(Lines[i].startpos.y);
		Verts.push_back(Lines[i].startpos.z);
		Verts.push_back(Lines[i].colour.x);
		Verts.push_back(Lines[i].colour.y);
		Verts.push_back(Lines[i].colour.z);
		//Lines[i].colour = glm::vec3(1, 0, 0);
		Verts.push_back(Lines[i].endpos.x);
		Verts.push_back(Lines[i].endpos.y);
		Verts.push_back(Lines[i].endpos.z);
		Verts.push_back(Lines[i].colour.x);
		Verts.push_back(Lines[i].colour.y);
		Verts.push_back(Lines[i].colour.z);
		//GLfloat vertices[] = {
		//	xpos,     ypos + h,   0.0  ,
		//	xpos,     ypos,       0.0 ,
		//	xpos + w, ypos,       1.0 ,

		//	xpos,     ypos + h,   0.0,
		//	xpos + w, ypos,       1.0 ,
		//	xpos + w, ypos + h,   1.0
		//};
		//Verts.push_back(vertices);
	}
	VertsOnGPU = Verts.size();
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * Verts.size(), &Verts[0], GL_DYNAMIC_DRAW);
}

void DebugLineDrawer::RenderLines(glm::mat4 matrix)
{
	if (VertsOnGPU != 0)
	{
		glDisable(GL_BLEND);
		m_TextShader->ActivateShaderProgram();

		//glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(UIManager::instance->GetWidth()), 0.0f, static_cast<GLfloat>(UIManager::instance->GetHeight()));
		glUniformMatrix4fv(glGetUniformLocation(m_TextShader->GetProgramHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(matrix));

		//glUniform3f(glGetUniformLocation(m_TextShader->GetProgramHandle(), "textColor"), Colour.x, Colour.y, Colour.z);

		GLsizei size = (6 * sizeof(GLfloat));
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size 2items
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			size,                  // stride
			(void*)0            // array buffer offset
		);
		//1>Rendering\Core\DebugLineDrawer.cpp(95): warning C4267: 'argument': conversion from 'size_t' to 'GLsizei', possible loss of data
		glEnableVertexAttribArray(0);
		glVertexAttribDivisor(1, 0);//particles did not clear 
		glVertexAttribPointer(
			1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size 
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			size,                  // stride
			(void*)(3 * sizeof(GLfloat))            // array buffer offset
		);

		glEnableVertexAttribArray(1);
		// Draw the triangles !
		glDrawArrays(GL_LINES, 0, (GLsizei)(Lines.size() * 2)); // 2*3 indices starting at 0 -> 2 triangles
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);
		glEnable(GL_DEPTH_TEST);
	}
	ClearLines();//bin all lines rendered this frame.
}

void DebugLineDrawer::ClearLines()
{
	for (int i = 0; i < Lines.size(); i++)
	{
		if (Lines[i].Time > 0)
		{
			Lines[i].Time -= PerfManager::GetDeltaTime();
		}
		else
		{
			Lines.erase(Lines.begin() + i);
		}
	}
	//Lines.clear();
}

void DebugLineDrawer::AddLine(glm::vec3 Start, glm::vec3 end, glm::vec3 colour, float time)
{
	WLine l;
	l.startpos = Start;
	l.endpos = end;
	l.colour = colour;
	l.Time = time;
	Lines.push_back(l);
}
