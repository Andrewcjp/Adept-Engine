#include "UIImage.h"
#include "UI/Core/UIDrawBatcher.h"
#include "UI/Core/UIWidgetContext.h"

UIImage::UIImage(int w, int h, int x, int y) :UIWidget(w, h, x, y)
{

}


UIImage::~UIImage()
{}


void UIImage::OnGatherBatches(UIRenderBatch* Groupbatchptr /*= nullptr*/)
{
	float xpos = (float)X;
	float ypos = (float)Y;
	const float h = mheight;
	const float w = mwidth;
	if (GetOwningContext() != nullptr)
	{
		UIRenderBatch* RenderBatch = nullptr;
		if (Groupbatchptr != nullptr && Groupbatchptr->BatchType == ERenderBatchType::TexturedVerts)
		{			
			RenderBatch = Groupbatchptr;
		}
		else
		{
			RenderBatch = new UIRenderBatch();
			RenderBatch->BatchType = ERenderBatchType::TexturedVerts;
		}
		RenderBatch->AddVertex(glm::vec2(xpos, ypos + h), true, glm::vec3(), glm::vec3(0, 0, 0));
		RenderBatch->AddVertex(glm::vec2(xpos, ypos), true, glm::vec3(), glm::vec3(0, 1, 0));
		RenderBatch->AddVertex(glm::vec2(xpos + w, ypos), true, glm::vec3(), glm::vec3(1, 1, 0));

		RenderBatch->AddVertex(glm::vec2(xpos, ypos + h), true, glm::vec3(), glm::vec3(0, 0, 0));
		RenderBatch->AddVertex(glm::vec2(xpos + w, ypos), true, glm::vec3(), glm::vec3(1, 1, 0));
		RenderBatch->AddVertex(glm::vec2(xpos + w, ypos + h), true, glm::vec3(), glm::vec3(1, 0, 0));
		RenderBatch->TextureInUse = TargetTexture;
		RenderBatch->RenderTarget = RenderTarget;
		if (Groupbatchptr == nullptr)
		{
			GetOwningContext()->GetBatcher()->AddBatch(RenderBatch);
		}
	}
}