#pragma once
#include "UI/Core/UIWidget.h"
#include "Core/Input/ITextInputReceiver.h"
class UIBox;
class UILabel;
class DebugConsole :public UIWidget, public ITextInputReceiver
{
public:
	DebugConsole(int w, int h, int  x, int  y);
	~DebugConsole();
	void Render() override;
	void Open();
	//void ResizeView(int w, int h, int x = 0, int y = 0)override;
	void Close();
	virtual void UpdateData() override;
	void UpdateSugestions();
	bool MatchStart(std::string A, std::string B);

	void ReceiveCommitedText(const std::string& text) override;
	std::string GetStartValue() override;
	void OnUpdate(const std::string & DisplayText) override;

private:
	void ExecCommand(std::string command);
	void ClearInput();
	bool IsOpen = false;
	std::string m_CurrentValue;
	std::string LastText;
	std::string LastCommand;
	UILabel* Textlabel;
	UILabel* ResponseLabel = nullptr;
	UILabel* SuggestBox = nullptr;
	UIBox* EditField;
	ConsoleVariable* CurrentTopCvar = nullptr;
};

