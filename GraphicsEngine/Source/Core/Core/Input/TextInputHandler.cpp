#include "TextInputHandler.h"
#include "ITextInputReceiver.h"
#include <cctype>
#include "Input.h"

TextInputHandler::TextInputHandler()
{}

TextInputHandler::~TextInputHandler()
{}

void TextInputHandler::SetInputContext(ITextInputReceiver * Target, MemberValueType::Type Filter/* = MemberValueType::String*/)
{
	m_pCurrentContext = Target;
	m_FilterType = Filter;
	if (Target != nullptr)
	{
		m_PreEditValue = Target->GetStartValue();
		m_CurrentValue = m_PreEditValue;
		m_CursorPos = 0;
		m_OldInputChannel = Input::Get()->GetCurrentChannel();
		Input::SetInputChannel(EInputChannel::TextInput);
	}
	ProcessKeyDown(0);
}

bool TextInputHandler::CheckValidInput(char c, MemberValueType::Type FilterType)
{
	if (FilterType == MemberValueType::String)
	{
		return true;
	}
	else if (FilterType == MemberValueType::Int || FilterType == MemberValueType::Float)
	{
		std::string Filter = "1234567890.";
		std::string IntFilter = "1234567890";
		if (FilterType == MemberValueType::Float)
		{
			return (Filter.find(c) != -1);
		}
		else if (FilterType == MemberValueType::Int)
		{
			return (IntFilter.find(c) != -1);
		}

	}
	return false;
}

void TextInputHandler::ProcessKeyDown(WPARAM key)
{
	if (!IsActive())
	{
		return;
	}
	if (key == VK_DELETE)
	{
		if (Input::GetKey(KeyCode::Shift, EInputChannel::TextInput))
		{
			m_CurrentValue = "";
		}
		else
		{
			m_CurrentValue.erase(m_CursorPos, 1);
			//m_CursorPos++;
		}
	}
	else if (key == VK_BACK)
	{
		if (Input::GetKey(KeyCode::Shift, EInputChannel::TextInput))
		{
			m_CurrentValue = "";
		}
		else if (m_CurrentValue.length() > 0 && m_CursorPos > 0)
		{
			m_CurrentValue.erase(m_CursorPos - 1, 1);
			if (m_CursorPos != m_CurrentValue.length())
			{
				m_CursorPos--;
			}
		}
	}
	else if (key == VK_RIGHT)
	{
		if (m_CursorPos < m_CurrentValue.length())
		{
			m_CursorPos++;
		}
	}
	else if (key == VK_LEFT)
	{
		if (m_CursorPos > 0)
		{
			m_CursorPos--;
		}
	}
	else  if (key == VK_RETURN)
	{
		AcceptValue();
		return;
	}
	else if (key == VK_ESCAPE)
	{
		RejectEdit();
		return;
	}
	else if (key != 0)
	{
		char c = (char)MapVirtualKey((UINT)key, MAPVK_VK_TO_CHAR);
		if (c != '\0')
		{
			if (!(GetKeyState(VK_LSHIFT) & 0x8000) && !(GetKeyState(VK_RSHIFT) & 0x8000))
			{
				c = (char)std::tolower(c);
			}
			if (CheckValidInput(c, m_FilterType))
			{
				m_CurrentValue.insert(m_CurrentValue.begin() + m_CursorPos, c);
				m_CursorPos++;
			}
		}
	}
	m_CursorPos = glm::clamp(m_CursorPos, 0, (int)m_CurrentValue.length());
	m_DisplayText = m_CurrentValue;

	if (m_DisplayText.length() > 0 && m_DisplayText.length() != m_CursorPos)
	{
		m_DisplayText.insert(m_CursorPos, "|");
	}
	else
	{
		m_DisplayText.append("|");
	}
	if (m_pCurrentContext->DisplayStartChar.size() > 0)
	{
		m_DisplayText.insert(m_DisplayText.begin(), m_pCurrentContext->DisplayStartChar[0]);
	}
	UpdateValue(m_DisplayText);
}

void TextInputHandler::AcceptValue()
{
	UpdateValue(m_PreEditValue);
	m_pCurrentContext->ReceiveCommitedText(m_CurrentValue);
	m_pCurrentContext = nullptr;
	Input::SetInputChannel(m_OldInputChannel);
}

void TextInputHandler::RejectEdit()
{
	//PreEditValue
	UpdateValue(m_PreEditValue);
	m_pCurrentContext->ReceiveCommitedText(m_PreEditValue);
	m_pCurrentContext = nullptr;
	Input::SetInputChannel(m_OldInputChannel);
}

TextInputHandler * TextInputHandler::Get()
{
	if (Input::Get() == nullptr)
	{
		return nullptr;
	}
	return Input::Get()->GetTextInputHandler();
}

void TextInputHandler::CommitValue()
{}

void TextInputHandler::UpdateValue(const std::string & newvalue)
{
	m_pCurrentContext->OnUpdate(m_DisplayText);
}
