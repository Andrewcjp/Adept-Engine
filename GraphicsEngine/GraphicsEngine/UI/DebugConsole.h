#pragma once
#include "UIWidget.h"
class UIBox;
class UILabel;
class DebugConsole :public UIWidget
{
public:
	DebugConsole(int w, int h, int  x, int  y);
	~DebugConsole();
	void Render() override;

	void ResizeView(int w, int h, int x = 0, int y = 0)override;
	void ExecCommand(std::string command);
	void Close();
	void ProcessKeyDown(UINT_PTR key) override;
private:
	bool IsOpen = false;
	std::string nextext;
	std::string LastText;
	std::string LastCommand;
	UILabel* Textlabel;
	UIBox* EditField;
};

