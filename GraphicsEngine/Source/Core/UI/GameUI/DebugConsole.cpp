#include "DebugConsole.h"
#include "Core/EngineInc.h"
#include "Core/Input/Input.h"

#include "Core/Platform/ConsoleVariable.h"
#include "Editor/EditorWindow.h"
#include "UI/Core/UILabel.h"
#include "UI/EditorUI/UIEditField.h"
#include "UI/UIManager.h"
#include "UIGraph.h"
#include <cctype>

DebugConsole::DebugConsole(int w, int h, int  x, int  y) :UIWidget(w, h, x, y)
{
	EditField = new UIBox(w, h, x, y);
	Textlabel = new UILabel(">", w, 30, x, y);
	SuggestBox = new UILabel("sugest", w, 30, x, y);
	ResponseLabel = new UILabel("", w, 30, x, y);
	Textlabel->TextScale = 0.3f;
	EditField->SetEnabled(false);
	LastText = ">";
	LastCommand = ">";
	Priority = 10;
}


DebugConsole::~DebugConsole()
{
	SafeDelete(EditField);
}

void DebugConsole::Render()
{
	if (IsOpen)
	{
		Textlabel->Render();
		ResponseLabel->Render();
		SuggestBox->Render();
	}
}

void DebugConsole::Open()
{
	IsOpen = true;
	UIManager::SetCurrentcontext(this);

	nextext = ">";
	EditField->SetEnabled(IsOpen);
	UIManager::UpdateBatches();
}

void DebugConsole::ResizeView(int w, int h, int x, int y)
{
	EditField->ResizeView(w, h, x, y);
	const int size = 40;
	Textlabel->ResizeView(w, size, x, y + 10);
	ResponseLabel->ResizeView(w, size, x, y);
	SuggestBox->ResizeView(w, size, x, y - 15);
}

static ConsoleVariable showgraph("ui.showgraph", 0, ECVarType::ConsoleAndLaunch);
void DebugConsole::ExecCommand(std::string command)
{
	Log::LogMessage("Exec: " + command);
	SuggestBox->SetText("");
	nextext = "";
	LastCommand = command;
	StringUtils::RemoveChar(command, ">");
	ConsoleVariable* Var = nullptr;
	std::string Response = "Command Unknown: " + command;
	if (ConsoleVariableManager::TrySetCVar(command, &Var))
	{
		Response = Var->GetName() + " " + Var->GetValueString();
	}
	ResponseLabel->SetText(Response);
	UIManager::instance->Graph->SetEnabled(showgraph.GetBoolValue());
	ClearInput();
}

void DebugConsole::Close()
{
	IsOpen = false;
	EditField->SetEnabled(false);
	UIManager::UpdateBatches();
	ClearInput();
	Textlabel->SetText(nextext);

	UIManager::SetCurrentcontext(nullptr);
}

void DebugConsole::ClearInput()
{
	nextext = ">";
	LastText = ">";
	Textlabel->SetText(nextext);
}

#ifdef PLATFORM_WINDOWS
#include "Core/MinWindows.h"
#endif
void DebugConsole::ProcessKeyDown(UINT_PTR key)
{
#ifdef PLATFORM_WINDOWS
	if (key == VK_DELETE)
	{
		nextext = ">";
	}
	else if (key == VK_RETURN)
	{
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
	else if (key == VK_TAB)
	{
		if (CurrentTopCvar != nullptr)
		{
			nextext = ">" + CurrentTopCvar->GetName();
		}
	}
	else
	{
		char c = (char)MapVirtualKey((UINT)key, MAPVK_VK_TO_CHAR);
		if (c == '`')
		{
			Close();
		}
		else if (c != 0)
		{
			nextext.append(1, (char)std::tolower(c));
		}
	}
	//todo: Cursor Movement
	Textlabel->SetText(nextext);
	UpdateSugestions();
#endif
}
void DebugConsole::UpdateData()
{
#ifdef PLATFORM_WINDOWS
	if (Input::GetKeyDown(VK_OEM_8))
	{
		Open();
	}
#endif
}
void DebugConsole::UpdateSugestions()
{
	const int maxsuggestions = 10;
	std::string output;
	int Count = 0;
	std::string cmd = nextext;
	cmd.erase(0, 1);
	for (int i = 0; i < ConsoleVariableManager::Instance->ConsoleVars.size(); i++)
	{
		ConsoleVariable* cv = ConsoleVariableManager::Instance->ConsoleVars[i];
		if (MatchStart(cmd, cv->GetName()))
		{
			if (Count == 0)
			{
				CurrentTopCvar = cv;
			}
			output.append(cv->GetName() + ", ");
			Count++;
			if (Count > maxsuggestions)
			{
				break;
			}
		}
	}
	SuggestBox->SetText(output);
}

bool DebugConsole::MatchStart(std::string A, std::string B)
{
	if (A.length() > B.length())
	{
		return false;
	}
	for (int i = 0; i < A.length(); i++)
	{
		if (A[i] != B[i])
		{
			return false;
		}
	}
	return true;
}