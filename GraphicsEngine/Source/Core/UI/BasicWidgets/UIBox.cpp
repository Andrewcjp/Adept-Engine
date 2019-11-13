#include "UIBox.h"
#include "UI/UIManager.h"
#include "../Core/UIDrawBatcher.h"
#include "../Core/UIWidget.h"
#include "../Core/UIWidgetContext.h"
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
	UIWidget::ResizeView(w, h, x, y);
	if (!IsActive)
	{
		return;
	}
	RightRect = CollisionRect(WidthOfCollisonRects * 2, h, x + (w - WidthOfCollisonRects), y);
	InvalidateTransform();
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

void UIBox::OnGatherBatches(UIRenderBatch* Groupbatchptr /*= nullptr*/)
{
	float xpos = (float)X;
	float ypos = (float)Y;
	const float h = mheight;
	const float w = mwidth;
	if (GetOwningContext() != nullptr)
	{
		UIRenderBatch* RenderBatch = nullptr;
		if (Groupbatchptr != nullptr)
		{
			RenderBatch = Groupbatchptr;
		}
		else
		{
			RenderBatch = new UIRenderBatch();
		}
		RenderBatch->AddVertex(glm::vec2(xpos, ypos + h), true, Colour, BackgoundColour);
		RenderBatch->AddVertex(glm::vec2(xpos, ypos), true, Colour, BackgoundColour);
		RenderBatch->AddVertex(glm::vec2(xpos + w, ypos), true, Colour, BackgoundColour);

		RenderBatch->AddVertex(glm::vec2(xpos, ypos + h), true, Colour, BackgoundColour);
		RenderBatch->AddVertex(glm::vec2(xpos + w, ypos), true, Colour, BackgoundColour);
		RenderBatch->AddVertex(glm::vec2(xpos + w, ypos + h), true, Colour, BackgoundColour);

		RenderBatch->AddVertex(glm::vec2(xpos + EdgeGap, ypos + h - EdgeGap), false, Colour, BackgoundColour);
		RenderBatch->AddVertex(glm::vec2(xpos + EdgeGap, ypos + EdgeGap), false, Colour, BackgoundColour);
		RenderBatch->AddVertex(glm::vec2(xpos + w - EdgeGap, ypos + EdgeGap), false, Colour, BackgoundColour);

		RenderBatch->AddVertex(glm::vec2(xpos + EdgeGap, ypos + h - EdgeGap), false, Colour, BackgoundColour);
		RenderBatch->AddVertex(glm::vec2(xpos + w - EdgeGap, ypos + EdgeGap), false, Colour, BackgoundColour);
		RenderBatch->AddVertex(glm::vec2(xpos + w - EdgeGap, ypos + h - EdgeGap), false, Colour, BackgoundColour);
		if (Groupbatchptr == nullptr)
		{
			GetOwningContext()->GetBatcher()->AddBatch(RenderBatch);
		}
	}
}
