
#include "EditorUI.h"
#include "UI/Core/UIWidgetContext.h"
#include "UI/UIManager.h"
#include "UI/Core/UIWidget.h"
#include "UI/Core/UITransform.h"
#include "UI/BasicWidgets/UIImage.h"
#include "Core/BaseWindow.h"

EditorUI::EditorUI()
{
	Instance = this;
	Init();
}


EditorUI::~EditorUI()
{}

void EditorUI::Init()
{	
	PlatformMenuBar Menu;
	Menu.SingleHandleFuncPTR = &EditorUI::HandleCMD;
	Menu.MenuName = "File";
	Menu.AddItem("Save");
	Menu.AddItem("Load");
	Menu.AddItem("Exit");
	WindowsWindow::AddMenuBar(Menu);

	UIManager::Get()->AddWidgetContext(new UIWidgetContext());
}

void EditorUI::SetViewPortSize()
{
	if (RHI::GetRenderSettings()->VRHMDMode == EVRHMDMode::Disabled)
	{
		if (RHI::GetRenderSettings()->Testmode == BBTestMode::Limit)
		{
			RHI::GetRenderSettings()->LockBackBuffer = true;
			RHI::GetRenderSettings()->LockedWidth = ViewPortImage->GetTransfrom()->GetTransfromedSize().x;
			RHI::GetRenderSettings()->LockedHeight = ViewPortImage->GetTransfrom()->GetTransfromedSize().y;
		}
	}
}

void EditorUI::HandleCMD(int index)
{

}

EditorUI* EditorUI::Instance = nullptr;
