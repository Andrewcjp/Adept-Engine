
#include "EditorUI.h"
#include "UI/Core/UIWidgetContext.h"
#include "UI/UIManager.h"
#include "UI/Core/UIWidget.h"
#include "UI/Core/UITransform.h"
#include "UI/BasicWidgets/UIImage.h"
#include "Core/BaseWindow.h"
#include "Editor/EditorWindow.h"
#include "Core/GameObject.h"
#include "Core/Assets/Scene.h"
#include "EditorSettingsMenu.h"

EditorUI::EditorUI()
{
	Instance = this;
	Init();
}

EditorUI::~EditorUI()
{}

void EditorUI::Init()
{	
#ifdef PLATFORM_WINDOWS

	FileMenu.SingleHandleFuncPTR = &EditorUI::HandleCMD;
	FileMenu.MenuName = "File";
	FileMenu.AddItem("New");
	FileMenu.AddItem("Save");
	FileMenu.AddItem("Load");
	FileMenu.AddItem("Settings");
	FileMenu.AddItem("Exit");
	PlatformWindow::AddMenuBar(FileMenu);

	GameObjectMenu.SingleHandleFuncPTR = &EditorUI::HandleCMD_GameObject;
	GameObjectMenu.MenuName = "Add GameObject";
	GameObjectMenu.AddItem("New Empty");
	GameObjectMenu.AddItem("New Cube");
	PlatformWindow::AddMenuBar(GameObjectMenu);
#endif
	Settings = new EditorSettingsMenu();
	context = new UIWidgetContext();
	UIManager::Get()->AddWidgetContext(context);
	Settings->Create(context);
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

void EditorUI::HandleCMD_GameObject(int index)
{
	GameObject* Object = new GameObject("GameObject");
	EditorWindow::GetInstance()->GetCurrentScene()->AddGameobjectToScene(Object);
	EditorWindow::GetInstance()->RefreshScene();
}

void EditorUI::HandleCMD(int index)
{
	if (index == 0)
	{
		EditorWindow::GetInstance()->NewScene();
	}
	else if (index == 3)
	{
		Instance->Settings->Open();
	}
	else if (index == 4)
	{
		Engine::RequestExit(0);
	}
}

EditorUI* EditorUI::Instance = nullptr;
