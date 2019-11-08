
#include "EditorUI.h"
#include "UI/Core/UIWidgetContext.h"
#include "UI/UIManager.h"


EditorUI::EditorUI()
{
	Init();
}


EditorUI::~EditorUI()
{}

void EditorUI::Init()
{
	UIManager::Get()->AddWidgetContext(new UIWidgetContext());
}
