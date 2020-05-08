#pragma once

class UIWindow;
class UIWidgetContext;
class EditorSettingsMenu
{
public:
	EditorSettingsMenu();
	void Create(UIWidgetContext* con);
	void Open();
	void Close();

	void CreateDefaultTab();

	void CreateVRSTab();
private:
	UIWindow* m_Window;
};

