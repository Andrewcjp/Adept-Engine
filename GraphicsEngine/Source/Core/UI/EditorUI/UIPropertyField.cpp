#include "UIPropertyField.h"
#include "UI/Core/UIWidget.h"
#include "UI/CompoundWidgets/UIEditField.h"

UIPropertyField::UIPropertyField()
{
	Namelabel = new UILabel("name ", 0, 0, 0, 0);
	if (UseBackGround)
	{
		BackGround = new UIBox(0, 0);
		AddChild(BackGround);
	}
	AddChild(Namelabel);
	Transform.SetStretchMode(EAxisStretch::Width);
	UpdateSize();
}

UIPropertyField::~UIPropertyField()
{}

void UIPropertyField::SetTarget(ClassReflectionNode* Node)
{
	TargetProperty = Node;
	UpdateType();
}

void UIPropertyField::UpdateScaled()
{
	UpdateSize();
	UIWidget::UpdateScaled();
}

void UIPropertyField::UpdateType()
{
	if (Field != nullptr)
	{
		RemoveChild(Field);
		SafeDelete(Field);
	}
	Field = new UIEditField(TargetProperty);
	Namelabel->SetText(TargetProperty->GetDisplayName());
	AddChild(Field);
	UpdateSize();
}
void UIPropertyField::UpdateSize()
{
	glm::ivec2 size = Transform.GetSizeRootSpace();
	Namelabel->SetRootSpaceSize(size.x / 2, size.y, 0, 0);
	if (Field != nullptr)
	{
		Field->SetRootSpaceSize(size.x / 2, size.y, size.x / 2, 0);
	}
	if (BackGround != nullptr)
	{
		BackGround->SetRootSpaceSize(size.x, size.y, 0, 0);
	}
}