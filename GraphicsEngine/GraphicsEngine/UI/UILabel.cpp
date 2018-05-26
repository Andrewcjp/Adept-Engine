#include "UILabel.h"
#include "UIManager.h"
#include "TextRenderer.h"
UILabel::UILabel(std::string  text, int w, int h, int x, int y) : UIWidget(w, h, x, y)
{
	MText = text;
	TextScale = TextDefaultScale;
}


UILabel::~UILabel()
{
}
//todo: Fit in box
void UILabel::Render()
{
	if (TextRenderer::instance != nullptr)
	{
		return;
		UIManager::instance->RenderTextToScreen(MText, (float)X +10/*+ (mwidth / 2)*/, (float)Y + ((mheight / 2.0f) - (TextScale )), TextScale,glm::vec3(1));
	}
}

void UILabel::ResizeView(int w, int h, int x, int y)
{
	mheight = h;
	mwidth = w;
	X = x;
	Y = y;
}
