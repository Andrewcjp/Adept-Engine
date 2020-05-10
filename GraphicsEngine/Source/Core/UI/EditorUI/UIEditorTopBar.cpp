#include "UIEditorTopBar.h"
#include "UI/CompoundWidgets/UIButton.h"
#include "Editor/EditorWindow.h"
#include "UI/Core/UIWidget.h"
#include "UI/CompoundWidgets/UIPanel.h"

UIEditorTopBar::UIEditorTopBar() :UITab()
{
#if WITH_EDITOR
	m_PlayButton = new UIButton(200, 50, 0, 500);
	m_PlayButton->BindTarget(std::bind(&EditorWindow::EnterPlayMode, EditorWindow::GetInstance()));
	m_PlayButton->SetText("Play");
	TabPanelArea->AddChild(m_PlayButton);

	m_StopButton = new UIButton(200, 50, 0, 500);
	m_StopButton->BindTarget(std::bind(&EditorWindow::ExitPlayMode, EditorWindow::GetInstance()));
	m_StopButton->SetText("Stop");
	TabPanelArea->AddChild(m_StopButton);

	m_EjectButton = new UIButton(200, 50, 0, 500);
	m_EjectButton->BindTarget(std::bind(&EditorWindow::Eject, EditorWindow::GetInstance()));
	m_EjectButton->SetText("Eject");
	TabPanelArea->AddChild(m_EjectButton);
#endif
}

void UIEditorTopBar::UpdateScaled()
{
	glm::ivec2 size = TabPanelArea->GetTransfrom()->GetSizeRootSpace();
	int Offset = 10;
	int ButtonSize = 75;
	int MidPoint = (size.x / 2) - ButtonSize / 2;
	m_PlayButton->SetRootSpaceSize(ButtonSize, size.y - Offset, MidPoint - ButtonSize, Offset / 2);
	m_StopButton->SetRootSpaceSize(ButtonSize, size.y - Offset, MidPoint, Offset / 2);
	m_EjectButton->SetRootSpaceSize(ButtonSize, size.y - Offset, MidPoint + ButtonSize, Offset / 2);
	UITab::UpdateScaled();
}
