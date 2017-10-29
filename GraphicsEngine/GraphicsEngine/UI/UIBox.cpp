#include "UIBox.h"
#include "OpenGL/OGLShaderProgram.h"
#include "glm\glm.hpp"
#include "../UI/UIManager.h"
#include "RHI/BaseTexture.h"
#include "RHI/RHI.h"
#include "../Rendering/Core/GPUStateCache.h"
#include "UIDrawBatcher.h"
UIBox::UIBox(int w, int h, int x, int y) : UIWidget(w, h, x, y)
{
	Init();
}
void UIBox::Init()
{
	m_TextShader = new OGLShaderProgram();
	m_TextShader->CreateShaderProgram();
	m_TextShader->AttachAndCompileShaderFromFile("UI_vs", SHADER_VERTEX);
	m_TextShader->AttachAndCompileShaderFromFile("UI_fs", SHADER_FRAGMENT);

	m_TextShader->BindAttributeLocation(0, "vertex");

	m_TextShader->BuildShaderProgram();
	m_TextShader->ActivateShaderProgram();
	//texture = RHI::CreateTexture("../asset/texture/UI/PanelTex.png");
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	/*if (texture != nullptr)
	{
		Colour = glm::vec3(1.0f);
	}
	else
	{
		Colour = BackgoundColour;
	}*/

	ResizeView(mwidth, mheight, X, Y);
}
void UIBox::ResizeView(int w, int h, int x, int y)
{
	if (!IsActive)
	{
		return;
	}
	RightRect = CollisionRect(WidthOfCollisonRects * 2, h, x + (w - WidthOfCollisonRects), y);
	GLfloat xpos = (GLfloat)x;
	GLfloat ypos = (GLfloat)y;
	float value = 1;
	

	//GLfloat vertices[] = {
	//	xpos,     ypos + h,   value ,value ,
	//	xpos,     ypos,      value ,value,
	//	xpos + w, ypos,       value ,value ,

	//	xpos,     ypos + h,   value ,value,
	//	xpos + w, ypos,       value ,value ,
	//	xpos + w , ypos + h,  value ,value,

	//		 xpos + gap,     ypos + h - gap,   0.0 ,0.0 ,
	//		 xpos + gap,     ypos + gap,      0.0 ,0.0,
	//		 xpos + w - gap, ypos + gap,       0.0 ,0 ,

	//		 xpos + gap,     ypos + h - gap,   0.0 ,0.0,
	//		 xpos + w - gap, ypos + gap,       0.0 ,0.0 ,
	//		 xpos + w - gap, ypos + h - gap,  0.0 ,0.0


	//};
	//glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	if (UIDrawBatcher::instance != nullptr)
	{
		UIDrawBatcher::instance->AddVertex(glm::vec2(xpos, ypos + h), true,Colour,BackgoundColour);
		UIDrawBatcher::instance->AddVertex(glm::vec2(xpos, ypos), true, Colour, BackgoundColour);
		UIDrawBatcher::instance->AddVertex(glm::vec2(xpos + w, ypos), true, Colour, BackgoundColour);

		UIDrawBatcher::instance->AddVertex(glm::vec2(xpos, ypos + h), true, Colour, BackgoundColour);
		UIDrawBatcher::instance->AddVertex(glm::vec2(xpos + w, ypos), true, Colour, BackgoundColour);
		UIDrawBatcher::instance->AddVertex(glm::vec2(xpos + w, ypos + h), true, Colour, BackgoundColour);

		UIDrawBatcher::instance->AddVertex(glm::vec2(xpos + EdgeGap, ypos + h - EdgeGap), false, Colour, BackgoundColour);
		UIDrawBatcher::instance->AddVertex(glm::vec2(xpos + EdgeGap, ypos + EdgeGap), false, Colour, BackgoundColour);
		UIDrawBatcher::instance->AddVertex(glm::vec2(xpos + w - EdgeGap, ypos + EdgeGap), false, Colour, BackgoundColour);

		UIDrawBatcher::instance->AddVertex(glm::vec2(xpos + EdgeGap, ypos + h - EdgeGap), false, Colour, BackgoundColour);
		UIDrawBatcher::instance->AddVertex(glm::vec2(xpos + w - EdgeGap, ypos + EdgeGap), false, Colour, BackgoundColour);
		UIDrawBatcher::instance->AddVertex(glm::vec2(xpos + w - EdgeGap, ypos + h - EdgeGap), false, Colour, BackgoundColour);
	}
}

UIBox::~UIBox()
{
	glDeleteBuffers(1, &quad_vertexbuffer);
	delete texture;
}

void UIBox::MouseMove(int x, int y)
{
	if (Resizeable)
	{
		if (RightRect.Contains(x, y))
		{
			if (Selected)
			{
				mwidth = x;
				ResizeView(mwidth, mheight, X, Y);
			}
		}
	}

}
void UIBox::MouseClick(int x, int y)
{
	if (Resizeable)
	{
		if (RightRect.Contains(x, y))
		{
			Selected = true;
		}
	}
}
void UIBox::MouseClickUp(int x, int y)
{
	if (Resizeable)
	{
		Selected = false;
	}
}
void UIBox::Render()
{
	return;
	//old slow way
	//glDisable(GL_BLEND);

	glEnable(GL_BLEND);
	m_TextShader->ActivateShaderProgram();
	if (texture != nullptr)
	{
		texture->Bind(0);
	}
	else
	{
		RHI::UnBindUnit(0);
	}

	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(UIManager::instance->GetWidth()), 0.0f, static_cast<GLfloat>(UIManager::instance->GetHeight()));
	glUniformMatrix4fv(glGetUniformLocation(m_TextShader->GetProgramHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform1i(glGetUniformLocation(m_TextShader->GetProgramHandle(), "UseColour"), (texture == nullptr));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		4,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);


	glUniform3f(glGetUniformLocation(m_TextShader->GetProgramHandle(), "textColor"), Colour.x, Colour.y, Colour.z);
	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, 12); // 2*3 indices starting at 0 -> 2 triangles

	glDisableVertexAttribArray(0);

}
