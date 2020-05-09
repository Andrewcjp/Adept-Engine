#include "UIWorldSettingsTab.h"
#include "EditorSettingsMenu.h"

UIWorldSettingsTab::UIWorldSettingsTab() :UITab()
{
	SetName("World");
	EditorSettingsMenu::AddCvar("r.renderscale", "Render Scale", this);
	ListItems = true;
}
