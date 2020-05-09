#include "UILabel.h"
#include "UI/UIManager.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "../Core/UIWidgetContext.h"
#include "../Core/UIDrawBatcher.h"


UILabel::UILabel(std::string  text, int w, int h, int x, int y) : UIWidget(w, h, x, y)
{
	MText = text;
	TextScale = TextDefaultScale;
	Colour = glm::vec3(1);
	//RenderWidgetBounds = true;
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
const int textsize = 48;
glm::ivec2 UILabel::CalulateTextSize()
{
	glm::ivec2 size;

	int Size = textsize;// glm::iround(textsize);
	size = glm::ivec2(Size*MText.length(), Size);
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
	int Size = (int)GetTransfrom()->GetTransfromedSize().y - offset * 2;
	glm::ivec2 RectSize = GetTransfrom()->GetTransfromedSize();
	glm::vec2 xScale = glm::vec2(RectSize) / glm::vec2(GlyphSize);

	Size = Math::Max(Size, 1);
	ScaleTarget = (float)(Size) / (float)glm::max(GlyphSize.y, GlyphSize.x);
	ScaleTarget = Math::Min(xScale.x, xScale.y);
	ScaleTarget = xScale.y;
	ScaleTarget = glm::min(TextScale, ScaleTarget);
	ScaleTarget = Math::Min(ScaleTarget, 5.0f);
	std::string TMPTEXt = MText;
#if 0
	if (GlyphSize.x > RectSize.x && RectSize.x != 0)
	{
		int OverHang = glm::floor((float)GlyphSize.x*ScaleTarget);
		float Value = ((OverHang - RectSize.x)) / (float)textsize;
		int Remove = glm::floor(Value);
		Remove = Math::Max(abs(Remove) - 1, 0);
		Remove = Math::Min(Remove, (int)MText.length());
		if (Remove > 0 && Value > 0)
		{
			TMPTEXt.erase(TMPTEXt.end() - (Remove), TMPTEXt.end());
		}
	}
#endif
	batch->AddText(TMPTEXt, glm::vec2(GetOwningContext()->Offset.x + (float)X + 5, GetOwningContext()->Offset.y + (float)Y + GetTransfrom()->GetTransfromedSize().y / 4), ScaleTarget, glm::vec3(1, 1, 1), GetTransfrom()->GetTransfromRect());
	if (Groupbatchptr == nullptr)
	{
		GetOwningContext()->GetBatcher()->AddBatch(batch);
	}
}
