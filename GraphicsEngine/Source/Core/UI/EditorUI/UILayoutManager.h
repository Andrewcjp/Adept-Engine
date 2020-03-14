#pragma once

class UIWidgetContext;
class UIWidget;
class UILayoutManager
{
public:
	UILayoutManager();
	~UILayoutManager();
	void UpdateSlotSizes();
	struct DockedSlot
	{
		glm::ivec2 Pos = glm::ivec2();
		glm::ivec2 Size  = glm::ivec2();
		glm::ivec2 GetSize();
		UIWidget* Widget = nullptr;
	};
	enum EDockedSlotAnchour
	{
		Left,
		Right,
		Bottom,
		Top,
		Centre,
		Limit
	};

	void Update();
	void PushToSlots();
	void SetWidget(EDockedSlotAnchour slot, UIWidget* widget);

	DockedSlot Slots[EDockedSlotAnchour::Limit];

	UIWidgetContext* Context = nullptr;
};

