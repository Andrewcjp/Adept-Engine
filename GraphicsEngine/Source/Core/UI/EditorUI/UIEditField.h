#pragma once
#include "UI/Core/UIWidget.h"
#include "UI/Core/UIBox.h"
#include "UI/Core/CollisionRect.h"
#include "Editor/Inspector.h"
class UILabel;
class UIEditField :
	public UIBox
{
public:
	UIEditField(int, int, int, int);
#if WITH_EDITOR
	UIEditField(Inspector::InspectorPropery * Targetprop);
#endif
	~UIEditField();
	void SetLabel(std::string lavel);
	void MouseMove(int x, int y) override;
	void GetValueText(std::string & string);
	bool MouseClick(int x, int y) override;
	void MouseClickUp(int x, int y) override;
	// Inherited via UIWidget
	void Render() override;
	void ResizeView(int w, int h, int x, int y) override;
	void ProcessKeyDown(UINT_PTR key)override;
	void SendValue();
	bool CheckValidInput(char c);

private:
	UILabel* Namelabel = nullptr;
	UILabel* Textlabel = nullptr;
	UIBox* TextBox = nullptr;
	class UIButton* Toggle = nullptr;
	glm::vec3 colour = glm::vec3(0.8f);
	CollisionRect Rect;
	CollisionRect ValueDrawChangeRect;
	std::string nextext;
	std::string LastText;
	bool WasSelected = false;
	EditValueType::Type FilterType = EditValueType::String;
	int CursorPos = 0;
	std::string DisplayText;
	bool IsEditing = false;
	void* Valueptr;
	bool Enabled = true;
	bool SupportsScroll = true;
	bool Scrolling = false;
	int startx = 0;
	float StartValue = 0.0f;
	float ScrollScale = 0.001f;
#if WITH_EDITOR
	Inspector::InspectorPropery Property;
#endif

};

