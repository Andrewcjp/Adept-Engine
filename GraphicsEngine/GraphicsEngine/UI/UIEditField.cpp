#include "stdafx.h"
#include "UIEditField.h"
#include "UILabel.h"
#include "Editor/EditorWindow.h"
#include <cctype>
#include "UIManager.h"
UIEditField::UIEditField(int w, int h, int x, int y) :UIBox(w, h, x, y)
{
	Colour = colour;
	Namelabel = new UILabel("name ", w / 2, h, x, y);
	Textlabel = new UILabel(" data", w / 2, h, x + w / 2, y);
	TextBox = new UIBox(w / 2, h, x + w / 2, y);
	TextBox->BackgoundColour = glm::vec3(0);
	TextBox->Colour = glm::vec3(0.3f);
	Rect = CollisionRect(w, h, x, y);
	Enabled = true;

}

UIEditField::UIEditField(Inspector::ValueType type, std::string name, void *valueptr) : UIEditField(0, 0, 0, 0)
{
	FilterType = type;
	Namelabel->SetText(name);
	Valueptr = valueptr;
	if (Valueptr != nullptr)
	{
		nextext = *((std::string*)Valueptr);
	}
	Textlabel->SetText(nextext);
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
	if (Rect.Contains(x, y))
	{
		UIManager::UpdateBatches();
		WasSelected = true;
		Input::Cursor = LoadCursor(NULL, IDC_IBEAM);
		Input::Cursor = SetCursor(Input::Cursor);

	}
	else
	{
		if (WasSelected)
		{
			//works currently but might leak resources;
			Input::Cursor = LoadCursor(NULL, IDC_ARROW);
			Input::Cursor = SetCursor(Input::Cursor);
			UIManager::UpdateBatches();
			WasSelected = false;
		}
	}

}

void UIEditField::MouseClick(int x, int y)
{
	if (Rect.Contains(x, y))
	{
		EditorWindow::SetContext(this);
		nextext = Textlabel->GetText();
		LastText = Textlabel->GetText();
		IsEditing = true;
		if (Valueptr != nullptr)
		{
			nextext = *((std::string*)Valueptr);
		}
		ProcessKeyDown(0);
	}
	else
	{
		if (EditorWindow::CurrentContext == this)
		{
			//if we are set un set us!
			EditorWindow::SetContext(nullptr);
			//	Textlabel->SetText(nextext);
			IsEditing = false;
		}
	}
}

void UIEditField::Render()
{
	UIBox::Render();
	if (FilterType != Inspector::Label)
	{
		TextBox->Render();
	}
	Namelabel->Render();
	Textlabel->Render();
}

void UIEditField::ResizeView(int w, int h, int x, int y)
{
	UIBox::ResizeView(w, h, x, y);
	Namelabel->TextScale = 0.3f;
	Textlabel->TextScale = 0.3f;
	Namelabel->ResizeView(w / 3, h / 2, x, y);
	int gap = 25;
	Textlabel->ResizeView(((w / 3) * 2) - gap, h / 2, x + (w / 3) + gap, y);
	if (FilterType != Inspector::Label)
	{
		TextBox->ResizeView(((w / 3) * 2) - gap, h, x + (w / 3) + gap, y);
	}
	else
	{
		Enabled = false;
	}
	Rect = CollisionRect(((w / 3) * 2) - gap, h, x + (w / 3) + gap, y);
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
		EditorWindow::SetContext(nullptr);
		Textlabel->SetText(nextext);
		IsEditing = false;
		SendValue();
	}
	else if (key == VK_ESCAPE)
	{
		nextext = LastText;
		IsEditing = false;
		Textlabel->SetText(nextext);
		EditorWindow::SetContext(nullptr);
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