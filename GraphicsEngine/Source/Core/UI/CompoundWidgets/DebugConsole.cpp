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
	TextInputHandler::Get()->SetInputContext(this, MemberValueType::String, false);
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
		TextInputHandler::Get()->AcceptValue(true);
	}

	SetEnabled(false);
}

void DebugConsole::ClearInput()
{
	m_CurrentValue = "";
	LastText = "";	
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
	if (IsOpen)
	{
		if (SuggestedVars.size() > 0)
		{
			if (Input::GetKeyDown(KeyCode::ArrowUp, EInputChannel::Global))
			{
				m_CurrentValue = SuggestedVars[TopCvar]->GetName();
				TopCvar++;
				DidJustMoveUp = true;
				TextInputHandler::Get()->SetInputContext(this, MemberValueType::String, false);
			}

			if (Input::GetKeyDown(KeyCode::ArrowDown, EInputChannel::Global))
			{
				m_CurrentValue = SuggestedVars[TopCvar]->GetName();
				TopCvar--;
				DidJustMoveUp = true;
				TextInputHandler::Get()->SetInputContext(this, MemberValueType::String, false);
			}
			TopCvar = TopCvar % SuggestedVars.size();
		}
		else { TopCvar = 0; }
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
	SuggestedVars.clear();
	TopCvar = 0;
	for (int i = 0; i < ConsoleVariableManager::Instance->ConsoleVars.size(); i++)
	{
		ConsoleVariable* cv = ConsoleVariableManager::Instance->ConsoleVars[i];
		if (MatchStart(cmd, cv->GetName()))
		{
			output.append(cv->GetName() + ", ");
			Count++;
			SuggestedVars.push_back(cv);
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
	TextInputHandler::Get()->SetInputContext(this, MemberValueType::String, false);
}

std::string DebugConsole::GetStartValue()
{
	return m_CurrentValue;
}

void DebugConsole::OnUpdate(const std::string & DisplayText)
{
	const bool CurentV = m_CurrentValue == TextInputHandler::Get()->GetCurrentValue();
	m_CurrentValue = TextInputHandler::Get()->GetCurrentValue();
	if (!DidJustMoveUp && !CurentV)
	{
		UpdateSugestions();
	}
	DidJustMoveUp = false;
	Textlabel->SetText(DisplayText);
}
