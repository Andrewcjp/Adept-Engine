#include "UIWidget.h"
#include "UI/UIManager.h"
#include "UI/Core/UIWidgetContext.h"
#include "UIDrawBatcher.h"
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

UIWidget::~UIWidget()
{}

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
	UpdateScaled();
}
void UIWidget::SetRootSpaceScaled(int w, int h, int x, int y)
{
	Transform.Set(w, h, x, y, EWidetSizeSpace::RootSpaceScaled);
	ScaleMode = EWidetSizeSpace::RootSpace;
	UpdateScaled();
}
void UIWidget::SetAbsoluteSize(int w, int h, int x, int y)
{
	Transform.Set(w, h, x, y, EWidetSizeSpace::ABS);
	ScaleMode = EWidetSizeSpace::ABS;
	UpdateScaled();
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
	ResizeView(Out.x, Out.y, Out.z, Out.w);
}

void UIWidget::SetOwner(UIWidgetContext * wc)
{
	SetOwningContext(wc);
	OnOwnerSet(wc);
}
void UIWidget::UpdateData()
{}
void UIWidget::MouseMove(int, int)
{}

bool UIWidget::MouseClick(int, int)
{
	return false;
}

void UIWidget::MouseClickUp(int, int)
{}

void UIWidget::PostBatchRender()
{}

void UIWidget::ProcessKeyDown(UINT_PTR)
{}
void UIWidget::ProcessKeyUp(UINT_PTR)
{}
void UIWidget::SetEnabled(bool state)
{
	UIManager::UpdateBatches();
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
		//return;
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
}

UITransform* UIWidget::GetTransfrom()
{
	return &Transform;
}

void UIWidget::OnGatherBatches(UIRenderBatch* Groupbatchptr /*= nullptr*/)
{

}

