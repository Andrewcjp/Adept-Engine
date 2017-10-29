#include "UILabel.h"
#include "OpenGL/OGLShaderProgram.h"
#include <GLEW\GL\glew.h>
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
		TextRenderer::instance->RenderFromAtlas(MText, (float)X +10/*+ (mwidth / 2)*/, (float)Y + ((mheight / 2.0f) - (TextScale * 10)), TextScale);
	}
}

void UILabel::ResizeView(int w, int h, int x, int y)
{
	mheight = h;
	mwidth = w;
	X = x;
	Y = y;
}
