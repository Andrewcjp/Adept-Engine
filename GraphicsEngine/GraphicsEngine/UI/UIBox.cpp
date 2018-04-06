#include "UIBox.h"
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

	if (UIDrawBatcher::instance != nullptr)
	{
		UIDrawBatcher::instance->AddVertex(glm::vec2(xpos, ypos + h), true, Colour, BackgoundColour);
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
void UIBox::MouseClickUp(int, int)
{
	if (Resizeable)
	{
		Selected = false;
	}
}
void UIBox::Render()
{
#if !USE_BATCHED_DRAW
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
#endif
}
