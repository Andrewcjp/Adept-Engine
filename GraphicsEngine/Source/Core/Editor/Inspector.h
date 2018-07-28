#pragma once
#include "UI/Core/UIWidget.h"
#include <vector>
class IInspectable;
class UIBox;
#include <functional>
class Inspector : public UIWidget
{
public:

	enum ValueType
	{
		Int,
		Float,
		String,
		Vector,
		Bool,
		Slider,
		Colour,
		Label
	};
	struct PType
	{
		ValueType type;
		union 
		{
			float fvalue;
			int ivalue;
		};
		union  
		{
			std::function<float()> fTarget;
			std::function<int()> iTarget;
		};
		~PType() {};
		PType() {};
		PType(PType&) {};
		PType operator=(PType t)
		{
			t.fTarget = fTarget;
			t.iTarget = iTarget;
			return t;
		}

	};
	struct PropertyField
	{
		
		PropertyField(ValueType t, PType ptype)
		{
			type = t;
			currentptype = ptype;
		}
		PType currentptype;
		ValueType type;
		PType GetValue()
		{
			if (currentptype.fTarget)
			{
				currentptype.fTarget();
			}
			return PType();
		};
		bool SetValue(PType value)
		{

		};
		class GameObject* ParentObject  = nullptr;
		~PropertyField() {};
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
	bool MouseClick(int x, int y);
	void MouseClickUp(int x, int y);
	void AddComponent();
	static void AddComponentCallback(int i);
private:
	static Inspector* Instance;
	void CreateEditor();
	float ItemHeight = 0.02f;
	IInspectable* target = nullptr;
	class UIButton* button;
	std::vector<UIWidget*> SubWidgets;
	UIBox* Backgroundbox;
};

