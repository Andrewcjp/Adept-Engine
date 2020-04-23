
#include "UIEditField.h"
#include "UI/BasicWidgets/UILabel.h"
#include "Editor/EditorWindow.h"
#include <cctype>
#include "UI/UIManager.h"
#include "UI/CompoundWidgets/UIButton.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Platform/Windows/WindowsWindow.h"
UIEditField::UIEditField(int w, int h, int x, int y) :UIBox(w, h, x, y)
{
	Colour = colour;
	Namelabel = new UILabel("name ", w / 2, h, x, y);
	Textlabel = new UILabel(" data", w / 2, h, x + w / 2, y);
	AddChild(Namelabel);
	AddChild(Textlabel);
	Rect = CollisionRect(w, h, x, y);
	Enabled = true;
}
#if WITH_EDITOR
UIEditField::UIEditField(Inspector::InspectorPropery* Targetprop) : UIEditField(0, 0, 0, 0)
{
	FilterType = Targetprop->type;
	Namelabel->SetText(Targetprop->name);
	Valueptr = Targetprop->ValuePtr;
	Property = *Targetprop;
	if (Targetprop->type == EditValueType::Bool)
	{
		Textlabel->SetEnabled(false);

	}
	else
	{
		Textlabel->SetText(nextext);
	}

	if (FilterType != EditValueType::Bool)
	{
		TextBox = new UIBox(0, 0, 0, 0);

		TextBox->BackgoundColour = glm::vec3(0.25f);
		TextBox->Colour = glm::vec3(0.7f);
		AddChild(TextBox);
	}
	else if (FilterType == EditValueType::Bool)
	{
		Toggle = new UIButton(0, 0, 0, 0);
		Toggle->BackgoundColour = glm::vec3(0.25f);
		Toggle->Colour = glm::vec3(0.7f);
		Toggle->BindTarget(std::bind(&UIEditField::SendValue, this));
		AddChild(Toggle);
	}
	SupportsScroll = (FilterType == EditValueType::Float);//todo: int
	if (Valueptr != nullptr)
	{
		GetValueText(nextext);
		Textlabel->SetText(nextext);
	}
	AddChild(Namelabel);
	AddChild(Textlabel);
}
#endif
UIEditField::~UIEditField()
{}

void UIEditField::SetLabel(std::string lavel)
{
	Namelabel->SetText(lavel);
}

void UIEditField::MouseMove(int x, int y)
{
	if (!Enabled)
	{
		return;
	}
	if (Scrolling)
	{
		if (Valueptr == nullptr)
		{
			Scrolling = false;
		}
		else
		{
			*((float*)Valueptr) = StartValue + ((startx - x)*ScrollScale);
			GetValueText(nextext);
			Textlabel->SetText(nextext);
			SendValue();
			return;
		}
	}
	if (FilterType == EditValueType::Bool)
	{
		Toggle->MouseMove(x, y);
		return;
	}
	if (TextBox->ContainsPoint(ConvertScreenToRootSpace(glm::ivec2(x, y))))
	{
		WasSelected = true;
		PlatformWindow::SetCursorType(GenericWindow::CursorType::IBeam);
	}
	else
	{

		if (WasSelected)
		{
			//works currently but might leak resources;			
			PlatformWindow::SetCursorType(GenericWindow::CursorType::Normal);
			WasSelected = false;
		}
	}


}
void UIEditField::GetValueText(std::string & string)
{
	if (FilterType == EditValueType::Float)
	{
		float t = *((float*)Valueptr);
		string = std::to_string(t);
	}
	if (FilterType == EditValueType::Bool)
	{
		Toggle->SetText(*((bool*)(Valueptr)) ? "True " : "False");
	}
}
bool UIEditField::MouseClick(int x, int y)
{
	if (FilterType == EditValueType::Bool)
	{
		Toggle->MouseClick(x, y);
		return false;
	}
	if (TextBox->ContainsPoint(ConvertScreenToRootSpace(glm::ivec2(x, y))))
	{
		UIManager::SetCurrentcontext(this);
		nextext = Textlabel->GetText();
		LastText = Textlabel->GetText();
		IsEditing = true;
		if (Valueptr != nullptr)
		{
			//nextext = *((std::string*)Valueptr);
			GetValueText(nextext);
		}
		ProcessKeyDown(0);
	}
	else
	{
		if (UIManager::GetCurrentContext() == this)
		{
			//if we are set un set us!
			UIManager::SetCurrentcontext(nullptr);
			//	Textlabel->SetText(nextext);
			IsEditing = false;
		}
	}
	if (ValueDrawChangeRect.Contains(x, y))
	{
		//__debugbreak();
		if (Valueptr != nullptr)
		{
			if (FilterType == EditValueType::Float)
			{
				Scrolling = true;
				startx = x;
				StartValue = *((float*)Valueptr);
			}
		}
	}
	return true;
}

void UIEditField::MouseClickUp(int x, int y)
{
	Scrolling = false;
}

void UIEditField::Render()
{
	UIBox::Render();
	if (FilterType != EditValueType::Label && FilterType != EditValueType::Bool)
	{
		TextBox->Render();
		Textlabel->Render();
	}
	if (FilterType == EditValueType::Bool)
	{
		Toggle->Render();
	}
	Namelabel->Render();
	if (Valueptr != nullptr && !IsEditing)
	{
		GetValueText(nextext);
		Textlabel->SetText(nextext);
	}
}

void UIEditField::SendValue()
{
	if (Valueptr != nullptr)
	{
		if (FilterType == EditValueType::String)
		{
			//nextext.copy((char*)Valueptr, nextext.length());
			//std::string Target = *((std::string*)Valueptr);
			//Target.clear();
			//Target.append(nextext);
			*((std::string*)Valueptr) = nextext;
		}
		else if (FilterType == EditValueType::Float)
		{
			float out = (float)atof(nextext.c_str());

			*((float*)Valueptr) = out;
		}
		else if (FilterType == EditValueType::Bool)
		{
			*((bool*)(Valueptr)) = !*((bool*)(Valueptr));
			Toggle->SetText(*((bool*)(Valueptr)) ? "True" : "False");
		}
		else if (FilterType == EditValueType::Int)
		{
			int out = (int)atoi(nextext.c_str());

			*((int*)Valueptr) = out;
		}
#if WITH_EDITOR
		if (Property.ChangesEditor)
		{
			UIManager::instance->RefreshGameObjectList();
		}
#endif
	}
}
#ifdef PLATFORM_WINDOWS
void UIEditField::ProcessKeyDown(WPARAM key)
{
	if (!Enabled)
	{
		return;
	}
	if (key == VK_DELETE)
	{
		nextext = "";
	}
	else if (key == VK_BACK)
	{
		if (nextext.length() > 0 && CursorPos > 0)
		{
			nextext.erase(CursorPos - 1, 1);
			if (CursorPos != nextext.length())
			{
				CursorPos--;
			}
		}
	}
	else if (key == VK_RIGHT)
	{
		if (CursorPos < nextext.length())
		{
			CursorPos++;
		}
	}
	else if (key == VK_LEFT)
	{
		if (CursorPos > 0)
		{
			CursorPos--;
		}
	}
	else  if (key == VK_RETURN)
	{
		UIManager::SetCurrentcontext(nullptr);
		Textlabel->SetText(nextext);
		IsEditing = false;
		SendValue();
	}
	else if (key == VK_ESCAPE)
	{
		nextext = LastText;
		IsEditing = false;
		Textlabel->SetText(nextext);
		UIManager::SetCurrentcontext(nullptr);
	}
	else if (key != 0)
	{

		char c = (char)MapVirtualKey((UINT)key, MAPVK_VK_TO_CHAR);
		if (!(GetKeyState(VK_LSHIFT) & 0x8000) && !(GetKeyState(VK_RSHIFT) & 0x8000))
		{
			c = (char)std::tolower(c);
		}
		if (CheckValidInput(c))
		{
			CursorPos++;
			nextext.append(1, c);
		}
	}
	CursorPos = glm::clamp(CursorPos, 0, (int)nextext.length());
	DisplayText = nextext;
	if (IsEditing)
	{
		if (DisplayText.length() > 0 && DisplayText.length() != CursorPos)
		{
			DisplayText.insert(CursorPos, "|");
		}
		else
		{
			DisplayText.append("|");
		}
	}
	//todo: Cursour Movement
	Textlabel->SetText(DisplayText);
}
#endif
bool UIEditField::CheckValidInput(char c)
{
	if (FilterType == EditValueType::String)
	{
		return true;
	}
	else if (FilterType == EditValueType::Int || FilterType == EditValueType::Float)
	{
		std::string Filter = "1234567890.";
		std::string IntFilter = "1234567890";
		if (FilterType == EditValueType::Float)
		{
			return (Filter.find(c) != -1);
		}
		else if (FilterType == EditValueType::Int)
		{
			return (IntFilter.find(c) != -1);
		}

	}
	return false;
}

void UIEditField::UpdateScaled()
{
	UIBox::UpdateScaled();
	int w = GetTransfrom()->GetSizeRootSpace().x;
	int h = GetTransfrom()->GetSizeRootSpace().y;

	//UIBox::ResizeView(w, h, x, y);
	Namelabel->TextScale = 0.3f;
	Textlabel->TextScale = 0.3f;
	int gap = 25;
	Namelabel->RenderWidgetBounds = true;
	Namelabel->SetRootSpaceSize((w / 3), h, 0, 0);
	Textlabel->SetRootSpaceSize((w / 3) + gap, h, (w / 3) + gap, 0);
	if (FilterType == EditValueType::Bool)
	{
		Toggle->SetRootSpaceSize(((w / 3) * 2) - gap, h, (w / 3) + gap, 0);
	}
	if (TextBox != nullptr && FilterType != EditValueType::Label)
	{
		TextBox->SetRootSpaceSize(((w / 3) * 2) - gap, h, (w / 3) + gap, 0);
	}
	else
	{
		Enabled = false;
	}
	Rect = CollisionRect(((w / 3) * 2) - gap, h, (w / 3) + gap, 0);
	//ValueDrawChangeRect = CollisionRect(w / 3, h, x, y);
}

void UIEditField::ProcessUIInputEvent(UIInputEvent& e)
{
	if (e.LeftMouse)
	{
		PlatformContextMenu Menu;
		Menu.AddItem("Play");
		Menu.AddItem("exit");
		uint Option = PlatformWindow::ShowContextMenu(Menu);
		e.SetHandled();
	}
}
