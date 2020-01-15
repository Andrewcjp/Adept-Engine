#pragma once
#include "UI/CompoundWidgets/UITab.h"

class UIListBox;
class EditorOutliner : public UITab
{
public:
	EditorOutliner();
	~EditorOutliner();
	void SetGameObjects(std::vector<GameObject*>& Objects);
	void UpdateScaled() override;
	void ReCreate();
	UIListBox* List = nullptr;
};

