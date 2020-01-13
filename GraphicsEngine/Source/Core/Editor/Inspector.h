#pragma once
#include "UI/Core/UIWidget.h"
#include <functional>
#include "Editor/EditorTypes.h"
#include "UI/CompoundWidgets/UITab.h"
#if WITH_EDITOR
class IEdtiorInspectable;
class UIBox;
struct InspectorProperyGroup;
class Inspector : public UITab
{
public:


	struct PType
	{
		EditValueType::Type type;

			float fvalue;
			int ivalue;
		
			std::function<float()> fTarget;
			std::function<int()> iTarget;
		
		~PType()
		{};
		PType()
		{};
		//PType(PType&)
		//{};
		//PType operator=(PType t)
		//{
		//	t.fTarget = fTarget;
		//	t.iTarget = iTarget;
		//	return t;
		//}

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
		class GameObject* ParentObject = nullptr;
		~PropertyField()
		{};
	};
	struct InspectorPropery
	{
		std::string name = "";
		void* ValuePtr = nullptr;
		EditValueType::Type type;
		bool ChangesEditor = false;
	};

	Inspector(int w, int h, int x, int y);
	~Inspector();
	void SetSelectedObject(IEdtiorInspectable* target);
	void Refresh();
	static InspectorPropery CreateProperty(std::string name, EditValueType::Type type, void* Valueptr, bool EditorEffect = false);
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
	std::vector<struct Inspector::InspectorPropery> SubProps;
};
#endif