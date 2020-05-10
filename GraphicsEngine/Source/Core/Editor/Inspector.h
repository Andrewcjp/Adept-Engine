#pragma once
#include "UI/Core/UIWidget.h"
#include <functional>
#include "Editor/EditorTypes.h"
#include "UI/CompoundWidgets/UITab.h"
#include "Core/Reflection/ClassReflectionNode.h"
#if WITH_EDITOR
class IEdtiorInspectable;
class UIBox;
struct InspectorProperyGroup;
class Inspector : public UITab
{
public:
	Inspector(int w, int h, int x, int y);
	~Inspector();
	void SetSelectedObject(IEdtiorInspectable* target);
	void Refresh();
	static InspectorProperyGroup CreatePropertyGroup(std::string name);
	void MouseMove(int x, int y);
	bool MouseClick(int x, int y);
	void MouseClickUp(int x, int y);
	void AddComponent();
	static void AddComponentCallback(int i);

	void UpdateScaled() override;

private:
	static Inspector* Instance;
	void CreateEditor();
	float ItemHeight = 0.01f;
	IEdtiorInspectable* target = nullptr;
	class UIButton* button;
};
struct InspectorProperyGroup
{
	std::string name;
	std::vector<ClassReflectionNode*> Nodes;
};
#endif