
#include "EditorUI.h"
#include "UI/Core/UIWidgetContext.h"
#include "UI/UIManager.h"
#include "UI/Core/UIWidget.h"
#include "UI/Core/UITransform.h"
#include "UI/BasicWidgets/UIImage.h"
#include "Core/BaseWindow.h"

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

void EditorUI::SetViewPortSize()
{
	RHI::GetRenderSettings()->LockBackBuffer = true;
	RHI::GetRenderSettings()->LockedWidth = ViewPortImage->GetTransfrom()->GetTransfromedSize().x;
	RHI::GetRenderSettings()->LockedHeight = ViewPortImage->GetTransfrom()->GetTransfromedSize().y;
}