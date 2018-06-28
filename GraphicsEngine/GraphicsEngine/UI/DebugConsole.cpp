#include "stdafx.h"
#include "DebugConsole.h"
#include "UI\UIEditField.h"
#include "Core/Input.h"
#include "UIManager.h"
#include "Editor/EditorWindow.h"
#include "UIBox.h"
#include "UILabel.h"
#include <cctype>
#include "UIGraph.h"
DebugConsole::DebugConsole(int w, int h, int  x, int  y) :UIWidget(w, h, x, y)
{
	EditField = new UIBox(w, h, x, y);
	Textlabel = new UILabel(">", w, 30, x, y);
	Textlabel->TextScale = 0.3f;
	EditField->SetEnabled(false);
	LastText = ">";
	LastCommand = ">";
	Priority = 10;
}


DebugConsole::~DebugConsole()
{
	delete EditField;
}

void DebugConsole::Render()
{
	if (Input::GetKeyDown(VK_OEM_8))
	{
		IsOpen = true;
		UIManager::SetCurrentcontext(this);

		nextext = ">";
		EditField->SetEnabled(IsOpen);
		UIManager::UpdateBatches();
	}
	if (IsOpen)
	{
		Textlabel->Render();
	}
}

void DebugConsole::ResizeView(int w, int h, int x, int y)
{
	EditField->ResizeView(w, 30, x, y);
	Textlabel->ResizeView(w, 30, x, y);
}

void DebugConsole::ExecCommand(std::string command)
{
	if (!BaseWindow::ProcessDebugCommand(command))
	{
		if (command.find("showgraph") != -1)
		{
			if (UIManager::instance)
			{
				UIManager::instance->Graph->SetEnabled(!UIManager::instance->Graph->GetEnabled());
			}
		}
	}
	LastCommand = command;
	Close();
}
void DebugConsole::Close()
{
	IsOpen = false;
	EditField->SetEnabled(false);
	UIManager::UpdateBatches();
	nextext = ">";
	LastText = ">";
	Textlabel->SetText(nextext);
	UIManager::SetCurrentcontext(nullptr);
}
void DebugConsole::ProcessKeyDown(UINT_PTR key)
{
	if (key == VK_DELETE)
	{
		nextext = ">";
	}
	else if (key == VK_RETURN)
	{
		UIManager::SetCurrentcontext(nullptr);
		ExecCommand(nextext);
	}
	else if (key == VK_ESCAPE)
	{
		nextext = LastText;
		UIManager::SetCurrentcontext(nullptr);
		Close();
	}
	else if (key == VK_BACK)
	{
		if (nextext.length() > 1)
		{
			nextext.erase(nextext.end() - 1);
		}
	}
	else if (key == VK_UP)
	{
		nextext = LastCommand;
	}
	else
	{
		char c = (char)MapVirtualKey((UINT)key, MAPVK_VK_TO_CHAR);
		if (c == '`')
		{
			Close();
		}
		else
		{
			nextext.append(1, (char)std::tolower(c));
		}
	}
	//todo: Cursour Movement
	Textlabel->SetText(nextext);
}
