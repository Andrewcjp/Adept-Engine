#include "DebugConsole.h"
#include "Core/EngineInc.h"
#include "Core/Input/Input.h"

#include "Core/Platform/ConsoleVariable.h"
#include "Editor/EditorWindow.h"
#include "UI/BasicWidgets/UILabel.h"
#include "UI/CompoundWidgets/UIEditField.h"
#include "UI/UIManager.h"
#include "UIGraph.h"
#include <cctype>
#include "Core/Input/TextInputHandler.h"

DebugConsole::DebugConsole(int w, int h, int  x, int  y) :UIWidget(w, h, x, y)
{
	EditField = new UIBox(w, h, x, y);
	Textlabel = new UILabel(">", w, h / 2, x, y);
	SuggestBox = new UILabel("", w, h / 2, x, y);
	ResponseLabel = new UILabel("", w, h / 2, x, y);
	EditField->SetRootSpaceSize(w, h, 0, 0);
	Textlabel->SetRootSpaceSize(w, h / 2, 0, 0);
	SuggestBox->SetRootSpaceSize(w, h / 2, 0, h / 2);
	ResponseLabel->SetRootSpaceSize(w, h / 2, 0, h / 2);
	Textlabel->GetTransfrom()->SetStretchMode(EAxisStretch::Width);
	EditField->GetTransfrom()->SetStretchMode(EAxisStretch::Width);
	SuggestBox->GetTransfrom()->SetStretchMode(EAxisStretch::Width);
	ResponseLabel->GetTransfrom()->SetStretchMode(EAxisStretch::Width);


	Textlabel->TextScale = 0.3f;
	EditField->SetEnabled(false);
	LastText = ">";
	LastCommand = ">";
	Priority = 1000;
	AddChild(EditField);
	AddChild(Textlabel);
	AddChild(SuggestBox);
	//AddChild(ResponseLabel);
	IgnoreboundsCheck = true;
	GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
	GetTransfrom()->SetStretchMode(EAxisStretch::Width);
	Close();
	BatchMode = EWidgetBatchMode::On;
	DisplayStartChar = ">";
}


DebugConsole::~DebugConsole()
{
	SafeDelete(EditField);
}

void DebugConsole::Render()
{

}

void DebugConsole::Open()
{
	IsOpen = true;

	m_CurrentValue = "";
	EditField->SetEnabled(IsOpen);
	SetEnabled(true);
	TextInputHandler::Get()->SetInputContext(this);
}

//void DebugConsole::ResizeView(int w, int h, int x, int y)
//{
//	EditField->ResizeView(w, h, x, y);
//	const int size = h / 2;
//	Textlabel->ResizeView(w, size, x, y + h / 3);
//	ResponseLabel->ResizeView(w, size, x, y);
//	SuggestBox->ResizeView(w, size, x, y);
//}

static ConsoleVariable showgraph("ui.showgraph", 0, ECVarType::ConsoleAndLaunch);
void DebugConsole::ExecCommand(std::string command)
{
	Log::LogMessage("Exec: " + command);
	SuggestBox->SetText("");
	m_CurrentValue = "";
	LastCommand = command;
	StringUtils::RemoveChar(command, ">");
	ConsoleVariable* Var = nullptr;
	std::string Response = "Command Unknown: " + command;
	if (ConsoleVariableManager::TrySetCVar(command, &Var))
	{
		Response = Var->GetName() + " " + Var->GetValueString();
	}
	SuggestBox->SetText(Response);
	UIManager::instance->Graph->SetEnabled(showgraph.GetBoolValue());
	ClearInput();
}

void DebugConsole::Close()
{
	IsOpen = false;
	EditField->SetEnabled(false);
	ClearInput();
	Textlabel->SetText(m_CurrentValue);
	if (TextInputHandler::Get()->IsUsing(this))
	{
		TextInputHandler::Get()->AcceptValue();
	}

	SetEnabled(false);
}

void DebugConsole::ClearInput()
{
	m_CurrentValue = "";
	LastText = "";
	Textlabel->SetText(m_CurrentValue);
}

#ifdef PLATFORM_WINDOWS
#include "Core/MinWindows.h"
#endif

void DebugConsole::UpdateData()
{
#ifdef PLATFORM_WINDOWS
	if (Input::GetKeyDown(KeyCode::Tidle, EInputChannel::Global))
	{
		if (IsOpen)
		{
			Close();
		}
		else
		{
			Open();
		}
	}
#endif
}
void DebugConsole::UpdateSugestions()
{
	const int maxsuggestions = 10;
	std::string output;
	int Count = 0;
	std::string cmd = m_CurrentValue;
	//cmd.erase(0, 1);
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

void DebugConsole::ReceiveCommitedText(const std::string& text)
{
	m_CurrentValue = text;
	ExecCommand(text);
}

std::string DebugConsole::GetStartValue()
{
	return m_CurrentValue;
}

void DebugConsole::OnUpdate(const std::string & DisplayText)
{
	m_CurrentValue = TextInputHandler::Get()->GetCurrentValue();
	UpdateSugestions();
	Textlabel->SetText(DisplayText);
}
