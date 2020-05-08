#include "EditorSettingsMenu.h"
#include "UI/CompoundWidgets/UIWindow.h"
#include "UI/CompoundWidgets/UITab.h"
#include "UI/Core/UIWidgetContext.h"

EditorSettingsMenu::EditorSettingsMenu()
{}

void EditorSettingsMenu::Create(UIWidgetContext* con)
{
	m_Window = new UIWindow();
	m_Window->SetRootSpaceSize(200, 500, 500, 300);
	con->AddWidget(m_Window);
	CreateDefaultTab();
	CreateVRSTab();
}

void EditorSettingsMenu::Open()
{
	m_Window->SetEnabled(true);
}

void EditorSettingsMenu::Close()
{
	m_Window->SetEnabled(false);
}

void EditorSettingsMenu::CreateDefaultTab()
{
	UITab* tab = new UITab();
	tab->SetName("General");
	m_Window->AddTab(tab);
}

void EditorSettingsMenu::CreateVRSTab()
{
	UITab* tab = new UITab();
	tab->SetName("VRS");
	m_Window->AddTab(tab);
}