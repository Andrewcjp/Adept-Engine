#include "EditorSettingsMenu.h"
#include "UI/CompoundWidgets/UIWindow.h"
#include "UI/CompoundWidgets/UITab.h"
#include "UI/Core/UIWidgetContext.h"
#include "UIPropertyField.h"
#include "UI/Core/UIWidget.h"
#include "UI/CompoundWidgets/UIPanel.h"
#include "Core/Reflection/ClassReflectionNode.h"

EditorSettingsMenu::EditorSettingsMenu()
{}

void EditorSettingsMenu::Create(UIWidgetContext* con)
{
	m_Window = new UIWindow();
	m_Window->SetRootSpaceSize(400, 500, 500, 300);
	m_Window->SetCloseable(true);
	m_Window->SetTitle("Settings");
	Close();
	con->AddWidget(m_Window);
	CreateDefaultTab();
	CreateRenderSettingTab();
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

void EditorSettingsMenu::AddCvar(std::string cvar, std::string DisplayName, UITab* tab)
{
	ConsoleVariable* CVar = ConsoleVariableManager::Get()->Find(cvar);
	UIPropertyField* Field = new UIPropertyField();
	if (CVar)
	{
		CVar->AccessReflection()->m_DisplayName = DisplayName;
		Field->SetTarget(CVar->AccessReflection());
		Field->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
	}
	tab->TabPanelArea->AddChild(Field);
}

void EditorSettingsMenu::CreateDefaultTab()
{
	UITab* tab = new UITab();

	tab->SetName("General");
	tab->ListItems = true;
	AddCvar("c.showbounds", "Show Object bounds", tab);
	AddCvar("c.freeze", "Freeze object culling", tab);
	m_Window->AddTab(tab);
}

void EditorSettingsMenu::CreateRenderSettingTab()
{
	UITab* tab = new UITab();

	tab->SetName("Render");
	tab->ListItems = true;
	RenderSettings* settings = RHI::GetRenderSettings();
	for (int i = 0; i < settings->AccessReflection()->Data.size(); i++)
	{
		UIPropertyField* Field = new UIPropertyField();
		Field->SetTarget(settings->AccessReflection()->Data[i]);
		Field->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
		tab->TabPanelArea->AddChild(Field);
	}
	m_Window->AddTab(tab);
}

void EditorSettingsMenu::CreateVRSTab()
{
	UITab* tab = new UITab();
	tab->SetName("VRS");
	m_Window->AddTab(tab);
}