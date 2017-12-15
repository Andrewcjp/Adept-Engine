#pragma once
#include "UIWidget.h"
#include "UIBox.h"
#include "CollisionRect.h"
#include "../Editor/Inspector.h"
class UILabel;
class UIEditField :
	public UIBox
{
public:
	UIEditField(int, int, int, int);
	UIEditField(Inspector::ValueType type, std::string name, void * valueptr);
	~UIEditField();
	void SetLabel(std::string lavel);
	void MouseMove(int x, int y) override;
	void MouseClick(int x, int y) override;
	// Inherited via UIWidget
	virtual void Render() override;
	void ResizeView(int w, int h, int x, int y) override;
	void ProcessKeyDown(UINT_PTR key)override;
	void SendValue();
	bool CheckValidInput(char c);
private:
	UILabel* Namelabel;
	UILabel* Textlabel;
	UIBox* TextBox;
	glm::vec3 colour = glm::vec3(0.8f);
	CollisionRect Rect;
	std::string nextext;
	std::string LastText;
	bool WasSelected = false;
	Inspector::ValueType FilterType = Inspector::ValueType::String;
	int CursorPos = 0;
	std::string DisplayText;
	bool IsEditing = false;
	void* Valueptr;
	bool Enabled = true;

};

