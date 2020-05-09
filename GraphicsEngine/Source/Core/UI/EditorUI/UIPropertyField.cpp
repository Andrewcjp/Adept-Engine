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
	if (TargetProperty->m_Type == MemberValueType::Vector3)
	{
		ExtraFields[0] = new UIEditField(TargetProperty);
		ExtraFields[0]->Targetcomponent = UIEditField::Y;
		ExtraFields[0]->Update();
		AddChild(ExtraFields[0]);
		ExtraFields[1] = new UIEditField(TargetProperty);
		ExtraFields[1]->Targetcomponent = UIEditField::Z;
		ExtraFields[1]->Update();
		AddChild(ExtraFields[1]);
		ExtraFieldCount = 2;
	}
	UpdateSize();
}
void UIPropertyField::UpdateSize()
{
	glm::ivec2 size = Transform.GetSizeRootSpace();
	Namelabel->SetRootSpaceSize(size.x / 2, size.y, 0, 0);
	const int HalfWidth = (size.x / 2);
	if (Field != nullptr)
	{
		if (TargetProperty->m_Type == MemberValueType::Bool)
		{
			int QSize = size.x / 4;
			Namelabel->SetRootSpaceSize(QSize * 3, size.y, 0, 0);
			Field->SetRootSpaceSize(QSize, size.y, QSize * 3, 0);
		}
		else if (TargetProperty->m_Type == MemberValueType::Vector3)
		{
			int Thirds = (size.x / 2) / 3;
			Field->SetRootSpaceSize(Thirds, size.y, HalfWidth, 0);
			for (int i = 0; i < ExtraFieldCount; i++)
			{
				int Spacing = 2 * i;
				ExtraFields[i]->SetRootSpaceSize(Thirds, size.y, HalfWidth + Spacing + (Thirds*(i + 1)), 0);
			}
		}
		else
		{
			Field->SetRootSpaceSize(size.x / 2, size.y, size.x / 2, 0);
		}
	}
	if (BackGround != nullptr)
	{
		BackGround->SetRootSpaceSize(size.x, size.y, 0, 0);
	}
}