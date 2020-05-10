#pragma once
#include "UI/Core/UIWidget.h"
#include "UI/BasicWidgets/UILabel.h"


class UIEditField;
class ClassReflectionNode;
class UIBox;
class UILabel;
class UIPropertyField : public UIWidget
{
public:
	UIPropertyField();
	~UIPropertyField();
	void SetTarget(ClassReflectionNode* Node);

	void UpdateScaled() override;
	bool UseBackGround = false;
	bool UseSilder = false;
protected:
	void UpdateType();
	void UpdateSize();
	UIEditField* Field = nullptr;
	UIEditField* ExtraFields[3]{};
	ClassReflectionNode* TargetProperty = nullptr;
	UILabel* Namelabel = nullptr;
	UIBox* BackGround = nullptr;
	int ExtraFieldCount = 0;
};

