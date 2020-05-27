#pragma once
#include "Core/Reflection/ClassReflectionNode.h"
#include "Input.h"

class ITextInputReceiver;
//handles keyboard input to text
class TextInputHandler
{
public:
	TextInputHandler();
	~TextInputHandler();
	void SetInputContext(ITextInputReceiver* Target, MemberValueType::Type Filter = MemberValueType::String,bool AccepyCloses = true);


	bool CheckValidInput(char c, MemberValueType::Type FilterType);
	void ProcessKeyDown(WPARAM key);
	bool IsActive() const { return m_pCurrentContext != nullptr; }
	bool IsUsing(ITextInputReceiver* context)const { return m_pCurrentContext == context; }
	void AcceptValue(bool force = false);
	void RejectEdit();
	static TextInputHandler* Get();
	std::string GetCurrentValue()const { return m_CurrentValue; }
	
private:
	void CommitValue();
	void UpdateValue(const std::string & newvalue);

	ITextInputReceiver* m_pCurrentContext = nullptr;
	MemberValueType::Type m_FilterType = MemberValueType::String;

	int m_CursorPos = 0;
	std::string m_DisplayText;
	std::string m_CurrentValue;
	std::string m_PreEditValue;
	EInputChannel::Type m_OldInputChannel = EInputChannel::Game;
	bool CloseOnSendValue = true;
};

