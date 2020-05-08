#pragma once
#include "UI/Core/UIWidget.h"
#include "UI/BasicWidgets/UIBox.h"
#include "UI/Core/CollisionRect.h"
#include "Editor/Inspector.h"
#include "Core/Reflection/ClassReflectionNode.h"
#include "Core/Input/ITextInputReceiver.h"
class UILabel;
class UIEditField :
	public UIBox, public ITextInputReceiver
{
public:
	UIEditField(int, int, int, int);
#if WITH_EDITOR
	UIEditField(ClassReflectionNode* Targetprop);
#endif
	~UIEditField();
	void SetLabel(std::string lavel);
	void MouseMove(int x, int y) override;
	void GetValueText(std::string & string);
	bool MouseClick(int x, int y) override;
	void MouseClickUp(int x, int y) override;
	// Inherited via UIWidget
	void Render() override;
#ifdef PLATFORM_WINDOWS
	void ProcessKeyDown(UINT_PTR key)override;
#endif
	void SendValue();
	bool CheckValidInput(char c);


	void UpdateScaled() override;


	void ProcessUIInputEvent(UIInputEvent& e) override;
	void ReceiveCommitedText(const std::string& text) override;
	std::string GetStartValue() override;
	void OnUpdate(const std::string & DisplayText) override;

private:

	UILabel* Textlabel = nullptr;
	UIBox* TextBox = nullptr;
	class UIButton* Toggle = nullptr;
	glm::vec3 colour = glm::vec3(0.8f);
	CollisionRect Rect;
	CollisionRect ValueDrawChangeRect;
	std::string nextext;
	std::string LastText;
	bool WasSelected = false;
	MemberValueType::Type FilterType = MemberValueType::Limit;
	EditValueType::Type EditMode = EditValueType::Value;
	int CursorPos = 0;
	std::string DisplayText;
	bool Enabled = true;
	bool SupportsScroll = true;
	bool Scrolling = false;
	int startx = 0;
	float StartValue = 0.0f;
	float ScrollScale = 0.001f;
#if WITH_EDITOR
	Inspector::InspectorPropery Property;
	ClassReflectionNode* Node = nullptr;
#endif

};

