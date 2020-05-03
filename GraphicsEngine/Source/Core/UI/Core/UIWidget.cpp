#include "UIWidget.h"
#include "UI/UIManager.h"
#include "UI/Core/UIWidgetContext.h"
#include "UIDrawBatcher.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Rendering/Core/Screen.h"
glm::vec3 DefaultColour = glm::vec3(1);
UIWidget::UIWidget(int w, int h, int x, int y)
{
	IsActive = true;
	mwidth = w;
	mheight = h;
	X = x;
	Y = y;
	Colour = glm::vec3(0.5f);
	SetOwningContext(UIManager::GetDefaultContext());
}

UIWidget::UIWidget()
{
	IsActive = true;
	Colour = glm::vec3(0.5f);
	SetOwningContext(UIManager::GetDefaultContext());
}

UIWidget::~UIWidget()
{
	if (Parent)
	{
		Parent->RemoveChild(this);
	}
}

void UIWidget::ResizeView(int w, int h, int x, int y)
{
	mwidth = w;
	mheight = h;
	X = x;
	Y = y;
}

void UIWidget::SetRootSpaceSize(int w, int h, int x, int y)
{
	Transform.Set(w, h, x, y, EWidetSizeSpace::RootSpace);
	ScaleMode = EWidetSizeSpace::RootSpace;
}

void UIWidget::SetRootSpaceScaled(int w, int h, int x, int y)
{
	Transform.Set(w, h, x, y, EWidetSizeSpace::RootSpaceScaled);
	ScaleMode = EWidetSizeSpace::RootSpace;
}

void UIWidget::SetAbsoluteSize(int w, int h, int x, int y)
{
	Transform.Set(w, h, x, y, EWidetSizeSpace::ABS);
	ScaleMode = EWidetSizeSpace::ABS;
}

void UIWidget::SetScaled(float Width, float height, float xoff, float yoff)
{
	int Rootx = OwningContext->RootSpaceViewport.Max.x *xoff;
	int Rooty = OwningContext->RootSpaceViewport.Max.y *yoff;
	int Rootwidth = OwningContext->RootSpaceViewport.Max.x *Width;
	int Rootheight = OwningContext->RootSpaceViewport.Max.y *height;

	Transform.Set(Rootwidth, Rootheight, Rootx, Rooty);
}

void UIWidget::UpdateScaled()
{
	glm::vec4 Out = Transform.GetTransfromRect();
	ResizeView((int)Out.x, (int)Out.y, (int)Out.z, (int)Out.w);
	for (int i = 0; i < Children.size(); i++)
	{
		Children[i]->UpdateScaled();
	}
	DebugRenderBounds();
}

void UIWidget::SetOwner(UIWidgetContext * wc)
{
	SetOwningContext(wc);
	OnOwnerSet(wc);
}
void UIWidget::UpdateData()
{}

bool UIWidget::ContainsPoint(glm::ivec2 Point)
{
	glm::ivec2 Size = GetTransfrom()->GetSizeRootSpace();
	glm::ivec2 Pos = GetTransfrom()->GetPositionForWidgetRootSpace();
	DebugColour = glm::vec3(1, 1, 1);
	if (Point.x > Pos.x && Point.x < (Pos.x + Size.x))
	{
		DebugColour = glm::vec3(1, 0, 0);
		if (Point.y > Pos.y  && Point.y < (Pos.y+ Size.y))
		{
			DebugColour = glm::vec3(0, 1, 0);
			return true;
		}
	}

	return false;
}

void UIWidget::MouseMove(int x, int y)
{
	for (int i = 0; i < Children.size(); i++)
	{
		Children[i]->MouseMove(x, y);
	}
}

bool UIWidget::MouseClick(int x, int y)
{
	for (int i = 0; i < Children.size(); i++)
	{
		Children[i]->MouseClick(x, y);
	}
	return false;
}

void UIWidget::MouseClickUp(int x, int y)
{
	for (int i = 0; i < Children.size(); i++)
	{
		Children[i]->MouseClickUp(x, y);
	}
}

void UIWidget::PostBatchRender()
{}

void UIWidget::ProcessKeyDown(UINT_PTR)
{}
void UIWidget::ProcessKeyUp(UINT_PTR)
{}
void UIWidget::SetEnabled(bool state)
{
	IsActive = state;
}
bool UIWidget::operator<(UIWidget* that) const
{
	return (this->Priority > that->Priority);
}

void UIWidget::InvalidateRenderstate()
{
	if (GetOwningContext())
	{
		GetOwningContext()->MarkRenderStateDirty();
	}
}

void UIWidget::GatherBatches(UIRenderBatch* BatchPtr)
{
	if (!IsActive)
	{
		return;
	}
	if (!IsWithinParentBounds())
	{
		return;
	}
	if (BatchMode == EWidgetBatchMode::On)
	{
		BatchPtr = new UIRenderBatch();
		BatchPtr->BatchType = ERenderBatchType::Verts;
		GetOwningContext()->GetBatcher()->AddBatch(BatchPtr);
	}
	OnGatherBatches(BatchPtr);
	for (int i = 0; i < Children.size(); i++)
	{
		Children[i]->GatherBatches(BatchPtr);
	}
}

struct rect
{
	int x;
	int y;
	int width;
	int height;
};

bool valueInRange(int value, int min, int max)
{
	return (value >= min) && (value <= max);
}

bool rectOverlap(rect A, rect B)
{
	bool xOverlap = valueInRange(A.x, B.x, B.x + B.width) ||
		valueInRange(B.x, A.x, A.x + A.width);

	bool yOverlap = valueInRange(A.y, B.y, B.y + B.height) ||
		valueInRange(B.y, A.y, A.y + A.height);

	return xOverlap && yOverlap;
}


bool UIWidget::IsWithinParentBounds()
{
	//if (mwidth == 0 && mheight == 0)
	//{
	//	return false;
	//}
	if (Parent == nullptr)
	{
		return true;
	}
	if (Parent->IgnoreboundsCheck)
	{
		return true;
	}
	rect Widget;
	Widget.x = X;
	Widget.y = Y;
	Widget.width = mwidth;
	Widget.height = mheight;

	rect parentrect;
	parentrect.x = Parent->X;
	parentrect.y = Parent->Y;
	parentrect.width = Parent->mwidth;
	parentrect.height = Parent->mheight;
	return rectOverlap(Widget, parentrect);
}

void UIWidget::AddChild(UIWidget * W)
{
	W->Parent = this;
	Children.push_back(W);
}

void UIWidget::RemoveChild(UIWidget* w)
{
	w->Parent = nullptr;
	VectorUtils::Remove(Children, w);
}

void UIWidget::RemoveAllChildren()
{
	for (int i = 0; i < Children.size(); i++)
	{
		Children[i]->Parent = nullptr;
		SafeDelete(Children[i]);
		
	}
	Children.clear();
}
void UIWidget::InvalidateTransform()
{
	TransfromDirty = true;
}

EWidgetBatchMode::Type UIWidget::GetBatchMode()
{
	if (Parent != nullptr && BatchMode == EWidgetBatchMode::Auto)
	{
		return Parent->BatchMode;
	}
	return BatchMode;
}

void UIWidget::SetOwningContext(UIWidgetContext* val)
{
	OwningContext = val;
	Transform.SetContext(val);
	Transform.Widget = this;
}

UITransform* UIWidget::GetTransfrom()
{
	return &Transform;
}

void UIWidget::DebugRenderBounds()
{
	if (!RenderWidgetBounds)
	{
		return;
	}
	glm::ivec2 Size = GetTransfrom()->GetTransfromedSize();
	glm::ivec2 Pos = GetTransfrom()->GetPositionForWidget();
	DebugColour = glm::vec3(1, 0, 0);
	DebugLineDrawer::Get2()->AddLine(glm::vec3(Pos.x, Pos.y, 0), glm::vec3(Pos.x + Size.x, Pos.y, 0), DebugColour);
	DebugLineDrawer::Get2()->AddLine(glm::vec3(Pos.x + Size.x, Pos.y, 0), glm::vec3(Pos.x + Size.x, Pos.y + Size.y, 0), DebugColour);

	DebugLineDrawer::Get2()->AddLine(glm::vec3(Pos.x, Pos.y + Size.y, 0), glm::vec3(Pos.x + Size.x, Pos.y + Size.y, 0), DebugColour);
	DebugLineDrawer::Get2()->AddLine(glm::vec3(Pos.x, Pos.y, 0), glm::vec3(Pos.x, Pos.y + Size.y, 0), DebugColour);
}

void UIWidget::ReceiveUIInputEvent(UIInputEvent & e)
{
	if (e.IsHandled())
	{
		return;
	}
	ProcessUIInputEvent(e);
	if (e.IsHandled())
	{
		return;
	}
	for (int i = 0; i < Children.size(); i++)
	{
		Children[i]->ReceiveUIInputEvent(e);
	}
}

void UIWidget::OnGatherBatches(UIRenderBatch* Groupbatchptr /*= nullptr*/)
{

}

glm::ivec2 UIWidget::ConvertScreenToRootSpace(glm::ivec2 In)
{
	glm::vec2 MouseScreen = (glm::vec2(In.x, In.y) / glm::vec2(Screen::GetWindowRes().x, Screen::GetWindowRes().y));
	MouseScreen.y = 1.0 - MouseScreen.y;
	MouseScreen *= glm::vec2(OwningContext->RootSpaceViewport.Max.x, OwningContext->RootSpaceViewport.Max.y);
	return glm::ivec2(MouseScreen.x, MouseScreen.y);
}