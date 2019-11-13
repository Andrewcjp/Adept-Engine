#include "UIImage.h"
#include "../Core/UIDrawBatcher.h"
#include "../Core/UIWidgetContext.h"


UIImage::UIImage(int w, int h, int x, int y) :UIWidget(w, h, x, y)
{}


UIImage::~UIImage()
{}

void UIImage::Render()
{

}

void UIImage::UpdateScaled()
{
	
}

void UIImage::OnGatherBatches(UIRenderBatch* Groupbatchptr /*= nullptr*/)
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
		RenderBatch->AddVertex(glm::vec2(xpos, ypos + h), true, Colour,glm::vec3());
		RenderBatch->AddVertex(glm::vec2(xpos, ypos), true, Colour, glm::vec3());
		RenderBatch->AddVertex(glm::vec2(xpos + w, ypos), true, Colour, glm::vec3());

		RenderBatch->AddVertex(glm::vec2(xpos, ypos + h), true, Colour, glm::vec3());
		RenderBatch->AddVertex(glm::vec2(xpos + w, ypos), true, Colour, glm::vec3());
		RenderBatch->AddVertex(glm::vec2(xpos + w, ypos + h), true, Colour, glm::vec3());

		if (Groupbatchptr == nullptr)
		{
			GetOwningContext()->GetBatcher()->AddBatch(RenderBatch);
		}
	}
}