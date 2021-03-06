#include "UITransform.h"
#include "UIWidgetContext.h"
#include "UI\UIManager.h"
#include "Rendering\Core\Screen.h"
#include "UIWidget.h"


void UITransform::Set(int width, int height, int x, int y, EWidetSizeSpace::Type SpaceMode)
{
	SetPos(IntPoint(x, y));
	SetSize(IntPoint(width, height));
	ScalingMode = SpaceMode;
}

void UITransform::SetPos(IntPoint val)
{
	Pos = val;
}

void UITransform::SetSize(IntPoint val)
{
	Size = val;
}

glm::vec2 UITransform::GetPositionForWidgetRootSpace()
{
	glm::vec2 AnchouredPos = glm::vec2(Pos.x, Pos.y);
	glm::ivec2 ParentMax = GetSizeRootSpace();
	if (Widget->Parent != nullptr)
	{
		ParentMax = Widget->Parent->GetTransfrom()->GetSizeRootSpace();
	}
	if (ScalingMode == EWidetSizeSpace::RootSpaceScaled)
	{
		const float ItemScale = Context->RootSpaceWidgetScale;
		AnchouredPos *= ItemScale;
		if (StretchMode == EAxisStretch::HeightRootSpace)
		{
			AnchouredPos = glm::vec2(Pos.x*ItemScale, Pos.y);
		}
		if (StretchMode == EAxisStretch::WidthRootSpace)
		{
			AnchouredPos = glm::vec2(Pos.x, Pos.y*ItemScale);
		}
	}
	if (AnchourPoint == EAnchorPoint::Top)
	{
		AnchouredPos.y = ParentMax.y - AnchouredPos.y - GetSizeRootSpace().y;
	}
	else if (AnchourPoint == EAnchorPoint::Right)
	{
		AnchouredPos.x = ParentMax.x - AnchouredPos.x;
	}
	if (Widget->Parent != nullptr)
	{
		AnchouredPos += Widget->Parent->GetTransfrom()->GetPositionForWidgetRootSpace();
	}
	return AnchouredPos;
}

glm::vec2 UITransform::GetPositionForWidget()
{
	glm::vec2 AnchouredPos = GetPositionForWidgetRootSpace();
	//scaling
	if (ScalingMode == EWidetSizeSpace::ABS)
	{
		return AnchouredPos;
	}
	else if (ScalingMode == EWidetSizeSpace::RootSpace || ScalingMode == EWidetSizeSpace::RootSpaceScaled)
	{
		glm::vec2 rootPos = Context->ConvertToNormalSpace(AnchouredPos);
		return glm::vec2(rootPos*glm::vec2(Screen::GetWindowRes()));
	}
	return glm::vec2();
}

int UITransform::GetWidthScaled(float v)
{
	if (ScalingMode == EWidetSizeSpace::RootSpace || ScalingMode == EWidetSizeSpace::RootSpaceScaled)
	{
		return Context->RootSpaceViewport.Max.x*v;
	}
	return UIManager::GetScaledWidth(1.0f);
}

int UITransform::GetHeightScaled(float v)
{
	if (ScalingMode == EWidetSizeSpace::RootSpace || ScalingMode == EWidetSizeSpace::RootSpaceScaled)
	{
		return Context->RootSpaceViewport.Max.y*v;
	}
	return UIManager::GetScaledHeight(v);
}

glm::vec2 UITransform::GetSizeRootSpace()
{
	glm::ivec2 ParentMax = glm::ivec2(0, 0);
	if (Widget->Parent != nullptr)
	{
		ParentMax = Widget->Parent->GetTransfrom()->GetSizeRootSpace();
	}
	else
	{
		ParentMax = glm::ivec2(Context->RootSpaceViewport.Max.x, Context->RootSpaceViewport.Max.y);
	}
	glm::vec2 RawSize = glm::vec2(Size.x, Size.y);
	if (ScalingMode == EWidetSizeSpace::RootSpaceScaled)
	{
		const float ItemScale = Context->RootSpaceWidgetScale;
		RawSize *= ItemScale;
		if (StretchMode == EAxisStretch::HeightRootSpace)
		{
			RawSize = glm::vec2(Size.x*ItemScale, Size.y);
		}
		if (StretchMode == EAxisStretch::WidthRootSpace)
		{
			RawSize = glm::vec2(Size.x, Size.y*ItemScale);
		}
	}
	if (StretchMode == EAxisStretch::Width || StretchMode == EAxisStretch::ALL)
	{
		//todo: more control over scaling scale?
		RawSize.x = ParentMax.x;
	}
	if (StretchMode == EAxisStretch::Height || StretchMode == EAxisStretch::ALL)
	{
		RawSize.y = ParentMax.y;
	}
	return RawSize;
}

glm::vec2 UITransform::GetTransfromedSize()
{
	glm::vec2 RawSize = GetSizeRootSpace();
	if (ScalingMode == EWidetSizeSpace::ABS)
	{
		return RawSize;
	}
	else if (ScalingMode == EWidetSizeSpace::RootSpace || ScalingMode == EWidetSizeSpace::RootSpaceScaled)
	{
		glm::vec2 rootSize = Context->ConvertToNormalSpace(RawSize);
		return glm::vec2(UIManager::GetScaledWidth(rootSize.x), UIManager::GetScaledHeight(rootSize.y));
	}
	return RawSize;
}

glm::vec4 UITransform::GetTransfromRect()
{
	return glm::vec4(GetTransfromedSize(), GetPositionForWidget());
}
