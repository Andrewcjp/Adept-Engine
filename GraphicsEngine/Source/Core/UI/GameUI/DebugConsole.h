#pragma once
#include "UI/Core/UIWidget.h"
class UIBox;
class UILabel;
class DebugConsole :public UIWidget
{
public:
	DebugConsole(int w, int h, int  x, int  y);
	~DebugConsole();
	void Render() override;

	void Open();

	void ResizeView(int w, int h, int x = 0, int y = 0)override;
	void ExecCommand(std::string command);
	void Close();
	void ProcessKeyDown(UINT_PTR key) override;

	virtual void UpdateData() override;

private:
	bool IsOpen = false;
	std::string nextext;
	std::string LastText;
	std::string LastCommand;
	UILabel* Textlabel;
	UIBox* EditField;
};

