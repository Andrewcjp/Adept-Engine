#pragma once

class UIWindow;
class UIWidgetContext;
class UITab;
class EditorSettingsMenu
{
public:
	EditorSettingsMenu();
	void Create(UIWidgetContext* con);
	void Open();
	void Close();

	static void AddCvar(std::string cvar, std::string DisplayName, UITab* tab);
	void CreateDefaultTab();

	void CreateVRSTab();
private:
	UIWindow* m_Window;
};

