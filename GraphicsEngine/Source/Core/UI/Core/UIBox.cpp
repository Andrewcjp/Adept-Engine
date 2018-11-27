#include "UIBox.h"
#include "UI/UIManager.h"
#include "UIDrawBatcher.h"
#include "UIWidget.h"
#include "UIWidgetContext.h"
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
	float xpos = (float)x;
	float ypos = (float)y;

	if (OwningContext != nullptr)
	{
		OwningContext->GetBatcher()->AddVertex(glm::vec2(xpos, ypos + h), true, Colour, BackgoundColour);
		OwningContext->GetBatcher()->AddVertex(glm::vec2(xpos, ypos), true, Colour, BackgoundColour);
		OwningContext->GetBatcher()->AddVertex(glm::vec2(xpos + w, ypos), true, Colour, BackgoundColour);

		OwningContext->GetBatcher()->AddVertex(glm::vec2(xpos, ypos + h), true, Colour, BackgoundColour);
		OwningContext->GetBatcher()->AddVertex(glm::vec2(xpos + w, ypos), true, Colour, BackgoundColour);
		OwningContext->GetBatcher()->AddVertex(glm::vec2(xpos + w, ypos + h), true, Colour, BackgoundColour);

		OwningContext->GetBatcher()->AddVertex(glm::vec2(xpos + EdgeGap, ypos + h - EdgeGap), false, Colour, BackgoundColour);
		OwningContext->GetBatcher()->AddVertex(glm::vec2(xpos + EdgeGap, ypos + EdgeGap), false, Colour, BackgoundColour);
		OwningContext->GetBatcher()->AddVertex(glm::vec2(xpos + w - EdgeGap, ypos + EdgeGap), false, Colour, BackgoundColour);

		OwningContext->GetBatcher()->AddVertex(glm::vec2(xpos + EdgeGap, ypos + h - EdgeGap), false, Colour, BackgoundColour);
		OwningContext->GetBatcher()->AddVertex(glm::vec2(xpos + w - EdgeGap, ypos + EdgeGap), false, Colour, BackgoundColour);
		OwningContext->GetBatcher()->AddVertex(glm::vec2(xpos + w - EdgeGap, ypos + h - EdgeGap), false, Colour, BackgoundColour);
	}
}

UIBox::~UIBox()
{}

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

bool UIBox::MouseClick(int x, int y)
{
	if (Resizeable)
	{
		if (RightRect.Contains(x, y))
		{
			Selected = true;
			return true;
		}
	}
	return false;
}

void UIBox::MouseClickUp(int, int)
{
	if (Resizeable)
	{
		Selected = false;
	}
}

void UIBox::Render()
{}
