#pragma once
#include "UI/Core/UIWidget.h"
#include "UI/BasicWidgets/UIBox.h"

class UITab;
class UILabel;
class UIPanel;
class UIWindow : public UIBox
{
public:
	UIWindow();

	void UpdateSize();

	~UIWindow();
	std::vector<UITab*> AttachedTabs;

	void UpdateScaled() override;
	void MouseMove(int x, int y) override;
	bool MouseClick(int x, int y) override;
	void MouseClickUp(int x, int y) override;
	void AddTab(UITab* Tab);
	void FocusTab(int index);
	bool GetIsFloating() const { return IsFloating; }
	void SetIsFloating(bool val);
	void SetCloseable(bool state);
	void SetTitle(const std::string& title);
protected:
	void CLose();
	bool IsCloseAble = false;
	bool IsFloating = false;
	UILabel* m_TitleLabel = nullptr;
	UIPanel* WorkArea = nullptr;
	UIButton* CloseButton = nullptr;
	void OnGatherBatches(UIRenderBatch* Groupbatchptr = nullptr) override;
	bool Drag = false;
	bool DidJustDrag = false;
	glm::ivec2 LastPos;
	int CurrentTab = 0;
	int TabSize = 30;
};

