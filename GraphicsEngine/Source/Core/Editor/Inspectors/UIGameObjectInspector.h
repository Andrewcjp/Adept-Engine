#pragma once
#include "Editor/UIInspectorBase.h"

class UIButton;
class UIGameObjectInspector: public UIInspectorBase
{
public:
	UIGameObjectInspector();
	static UIGameObjectInspector* Get();
	void SetSelectedObject(IEdtiorInspectable * newTarget);

protected:
	void OnPopulateFields() override;
	void OnInspectorFinishCreate(UIWidget* TabArea) override;
	void AddComponentCallback(int i);
	void AddComponent();
	static UIGameObjectInspector* Instance;
	UIButton *button = nullptr;
	std::vector<uint64> ListIds;
	IEdtiorInspectable* target = nullptr;
};

