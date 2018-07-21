#include "stdafx.h"
#include "UIEditField.h"
#include "UI/Core/UILabel.h"
#include "Editor/EditorWindow.h"
#include <cctype>
#include "UI/UIManager.h"
#include "UI/Core/UIButton.h"
#include "../Core/Platform/PlatformCore.h"
UIEditField::UIEditField(int w, int h, int x, int y) :UIBox(w, h, x, y)
{
	Colour = colour;
	Namelabel = new UILabel("name ", w / 2, h, x, y);
	Textlabel = new UILabel(" data", w / 2, h, x + w / 2, y);

	Rect = CollisionRect(w, h, x, y);
	Enabled = true;

}

UIEditField::UIEditField(Inspector::ValueType type, std::string name, void *valueptr) : UIEditField(0, 0, 0, 0)
{
	FilterType = type;
	Namelabel->SetText(name);
	Valueptr = valueptr;

	if (type == Inspector::ValueType::Bool)
	{
		Textlabel->SetEnabled(false);

	}
	else
	{
		Textlabel->SetText(nextext);
	}

	if (FilterType != Inspector::ValueType::Bool)
	{
		TextBox = new UIBox(0, 0, 0, 0);

		TextBox->BackgoundColour = glm::vec3(0.25f);
		TextBox->Colour = glm::vec3(0.7f);
	}
	else if (FilterType == Inspector::ValueType::Bool)
	{
		Toggle = new UIButton(0, 0, 0, 0);
		Toggle->BackgoundColour = glm::vec3(0.25f);
		Toggle->Colour = glm::vec3(0.7f);
		Toggle->BindTarget(std::bind(&UIEditField::SendValue, this));
	}
	SupportsScroll = (FilterType == Inspector::ValueType::Float);//todo: int
	if (Valueptr != nullptr)
	{
		GetValueText(nextext);
		Textlabel->SetText(nextext);
	}
}

UIEditField::~UIEditField()
{
}

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
	if (FilterType == Inspector::ValueType::Bool)
	{
		Toggle->MouseMove(x, y);
		return;
	}
	if (Rect.Contains(x, y))
	{
		UIManager::UpdateBatches();
		WasSelected = true;
		PlatformWindow::SetCursorType(GenericWindow::CursorType::IBeam);

	}
	else
	{
		
		if (WasSelected)
		{
			//works currently but might leak resources;
			UIManager::UpdateBatches();
			PlatformWindow::SetCursorType(GenericWindow::CursorType::Normal);
			WasSelected = false;
		}
	}


}
void UIEditField::GetValueText(std::string & string)
{
	if (FilterType == Inspector::ValueType::Float)
	{
		float t = *((float*)Valueptr);
		string = std::to_string(t);
	}
	if (FilterType == Inspector::ValueType::Bool)
	{
		Toggle->SetText(*((bool*)(Valueptr)) ? "True " : "False");
	}
}
bool UIEditField::MouseClick(int x, int y)
{
	if (FilterType == Inspector::ValueType::Bool)
	{
		Toggle->MouseClick(x, y);
		return false;
	}
	if (Rect.Contains(x, y))
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
			if (FilterType == Inspector::ValueType::Float)
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
	if (FilterType != Inspector::ValueType::Label && FilterType != Inspector::ValueType::Bool)
	{
		TextBox->Render();
		Textlabel->Render();
	}
	if (FilterType == Inspector::ValueType::Bool)
	{
		Toggle->Render();
	}
	Namelabel->Render();
	if (Valueptr != nullptr)
	{
		GetValueText(nextext);
		Textlabel->SetText(nextext);
	}
}

void UIEditField::ResizeView(int w, int h, int x, int y)
{
	//UIBox::ResizeView(w, h, x, y);
	Namelabel->TextScale = 0.3f;
	Textlabel->TextScale = 0.3f;
	Namelabel->ResizeView(w / 3, h / 2, x, y);
	int gap = 25;
	Textlabel->ResizeView(((w / 3) * 2) - gap, h / 2, x + (w / 3) + gap, y);
	if (FilterType == Inspector::ValueType::Bool)
	{
		Toggle->ResizeView(((w / 3) * 2) - gap, h, x + (w / 3) + gap, y);
	}
	if (TextBox != nullptr && FilterType != Inspector::Label)
	{
		TextBox->ResizeView(((w / 3) * 2) - gap, h, x + (w / 3) + gap, y);
	}
	else
	{
		Enabled = false;
	}
	Rect = CollisionRect(((w / 3) * 2) - gap, h, x + (w / 3) + gap, y);
	ValueDrawChangeRect = CollisionRect(w / 3, h, x, y);
}

void UIEditField::SendValue()
{
	if (Valueptr != nullptr)
	{
		if (FilterType == Inspector::ValueType::String)
		{
			//nextext.copy((char*)Valueptr, nextext.length());
			//std::string Target = *((std::string*)Valueptr);
			//Target.clear();
			//Target.append(nextext);
			*((std::string*)Valueptr) = nextext;
		}
		if (FilterType == Inspector::ValueType::Float)
		{
			float out = (float)atof(nextext.c_str());

			*((float*)Valueptr) = out;
		}
		if (FilterType == Inspector::ValueType::Bool)
		{
			*((bool*)(Valueptr)) = !*((bool*)(Valueptr));
			Toggle->SetText(*((bool*)(Valueptr)) ? "True" : "False");
		}
		UIManager::instance->RefreshGameObjectList();
	}
}

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
bool UIEditField::CheckValidInput(char c)
{
	if (FilterType == Inspector::String)
	{
		return true;
	}
	else if (FilterType == Inspector::Int || FilterType == Inspector::Float)
	{
		std::string Filter = "1234567890.";
		std::string IntFilter = "1234567890";
		if (FilterType == Inspector::Float)
		{
			return (Filter.find(c) != -1);
		}
		else if (FilterType == Inspector::Int)
		{
			return (IntFilter.find(c) != -1);
		}

	}
	return false;
}