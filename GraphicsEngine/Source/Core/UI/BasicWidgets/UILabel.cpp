#include "UILabel.h"
#include "UI/UIManager.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "../Core/UIWidgetContext.h"
#include "../Core/UIDrawBatcher.h"
#include "Core/Maths/Math.h"

UILabel::UILabel(std::string  text, int w, int h, int x, int y) : UIWidget(w, h, x, y)
{
	MText = text;
	TextScale = TextDefaultScale;
	Colour = glm::vec3(1);
}


UILabel::~UILabel()
{}

void UILabel::Render()
{}

void UILabel::ResizeView(int w, int h, int x, int y)
{
	mheight = h;
	mwidth = w;
	X = x;
	Y = y;
}

void UILabel::SetText(const std::string& text)
{
	MText = text;
}

std::string UILabel::GetText()
{
	return MText;
}

glm::ivec2 UILabel::CalulateTextSize()
{
	glm::ivec2 size;
	const int textsize = 48;
	int Size = glm::iround(textsize * TextScale);
	size = glm::ivec2(Size, Size*MText.length());
	return size;
}

void UILabel::OnGatherBatches(UIRenderBatch* Groupbatchptr /*= nullptr*/)
{
	if (MText.length() == 0)
	{
		return;
	}
	UIRenderBatch* batch = Groupbatchptr;
	if (batch == nullptr)
	{
		batch = new UIRenderBatch();
	}
	glm::ivec2 GlyphSize = CalulateTextSize();
	float ScaleTarget = TextScale;
	const int offset = 1;
	const int textsize = 48;
	int Size = GetTransfrom()->GetTransfromedSize().y - offset * 2;
	Size = Math::Max(Size, 1);
	ScaleTarget = (float)(Size) / (float)textsize;

	batch->AddText(MText, glm::vec2(GetOwningContext()->Offset.x + (float)X + 10, GetOwningContext()->Offset.y + (float)Y + 2), ScaleTarget);
	if (Groupbatchptr == nullptr)
	{
		GetOwningContext()->GetBatcher()->AddBatch(batch);
	}
}
