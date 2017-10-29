#include "UIWidget.h"
#include "../UI/UIManager.h"
glm::vec3 DefaultColour = glm::vec3(1);
UIWidget::UIWidget(int w, int h, int x, int y)
{
	IsActive = true;
	mwidth = w;
	mheight = h;
	X = x;
	Y = y;
	Colour = glm::vec3(0.5f);
}

UIWidget::~UIWidget()
{
}

void UIWidget::ResizeView(int w, int h, int x, int y)
{
	mwidth = w;
	mheight = h;
	X = x;
	Y = y;
}
void UIWidget::SetScaled(float Width, float height, float xoff, float yoff)
{
	UseScaled = true;
	WidthScale = Width;
	HeightScale = height;
	XoffsetScale = xoff;
	YoffsetScale = yoff;
}
void UIWidget::UpdateScaled()
{
	if (UseScaled)
	{
		ResizeView(UIManager::GetScaledWidth(WidthScale), UIManager::GetScaledHeight(HeightScale),
			UIManager::GetScaledWidth(XoffsetScale), UIManager::GetScaledHeight(YoffsetScale));
	}
}
void UIWidget::UpdateData()
{
}
void UIWidget::MouseMove(int x, int y)
{
}

void UIWidget::MouseClick(int x, int y)
{
}

void UIWidget::MouseClickUp(int x, int y)
{
}

void UIWidget::PostBatchRender()
{
}

void UIWidget::ProcessKeyDown(UINT_PTR key)
{
}
