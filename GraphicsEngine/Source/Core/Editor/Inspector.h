#pragma once
#include "UI/Core/UIWidget.h"
#include <vector>
#include <functional>
namespace EditValueType
{
	enum Type
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
};
#if WITH_EDITOR
class IInspectable;
class UIBox;

class Inspector : public UIWidget
{
public:

	
	struct PType
	{
		EditValueType::Type type;
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
		
		PropertyField(EditValueType::Type t, PType ptype)
		{
			type = t;
			currentptype = ptype;
		}
		PType currentptype;
		EditValueType::Type type;
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
		std::string name = "";
		void* ValuePtr = nullptr;
		EditValueType::Type type;
		bool ChangesEditor = false;
	};
	struct InspectorProperyGroup
	{
		std::string name;
		std::vector<InspectorPropery> SubProps;
	};
	Inspector(int w, int h, int x, int y);
	~Inspector();
	void SetSelectedObject(IInspectable* target);
	void Refresh();
	static InspectorPropery CreateProperty(std::string name, EditValueType::Type type, void* Valueptr, bool EditorEffect = false);
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

#endif