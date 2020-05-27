#pragma once
#include "UI/Core/UIWidget.h"
#include <functional>
#include "Editor/EditorTypes.h"
#include "UI/CompoundWidgets/UITab.h"
#include "Core/Reflection/ClassReflectionNode.h"
#if WITH_EDITOR
class IEdtiorInspectable;
class UIBox;
struct InspectorProperyGroup
{
	std::string name;
	std::vector<ClassReflectionNode*> Nodes;
};
class UIInspectorBase : public UITab
{
public:
	UIInspectorBase(int w, int h, int x, int y);
	~UIInspectorBase();
	void Refresh();
	static InspectorProperyGroup CreatePropertyGroup(std::string name);
	void MouseMove(int x, int y);
	bool MouseClick(int x, int y);
	void MouseClickUp(int x, int y);


	void UpdateScaled() override;
protected:
	virtual void OnPopulateFields(){}
	std::vector<InspectorProperyGroup> Fields;
	virtual void OnInspectorFinishCreate(UIWidget* TabArea) {};
	void CreateEditor();
private:
	float ItemHeight = 0.01f;
};

#endif