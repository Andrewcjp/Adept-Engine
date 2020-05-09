#pragma once
#include "UI/CompoundWidgets/UITab.h"
class UIEditorTopBar : public UITab
{
public:
	UIEditorTopBar();

	void UpdateScaled() override;
private:
	UIButton* m_PlayButton = nullptr;
	UIButton* m_StopButton = nullptr;
	UIButton* m_EjectButton = nullptr;
};

