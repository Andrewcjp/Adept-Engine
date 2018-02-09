#pragma once
#include "UI/UIWidget.h"
#include <vector>
class IInspectable;
class UIBox;
class Inspector : public UIWidget
{
public:
	
	enum ValueType
	{
		Int,
		Float,
		String,
		Vector,
		Other,
		Label
	};
	struct InspectorPropery
	{
		std::string name;
		void* ValuePtr;
		ValueType type;
	};
	struct InspectorProperyGroup
	{
		std::string name;
		std::vector<InspectorPropery> SubProps;
	};
	Inspector(int w, int h, int x, int y);
	~Inspector();
	void SetSelectedObject(IInspectable* target);
	static InspectorPropery CreateProperty(std::string name, ValueType type, void* Valueptr);
	static InspectorProperyGroup CreatePropertyGroup(std::string name);
	// Inherited via UIWidget
	void Render() override;
	void ResizeView(int, int, int, int);
	void MouseMove(int x, int y);
	void MouseClick(int x, int y);
	void MouseClickUp(int x, int y);
	void AddComponent();
	static void AddComponentCallback(int i);
private:
	static Inspector* Instance;
	void CreateEditor();
	IInspectable* target;
	class UIButton* button;
	std::vector<UIWidget*> SubWidgets;
	UIBox* Backgroundbox;
};

