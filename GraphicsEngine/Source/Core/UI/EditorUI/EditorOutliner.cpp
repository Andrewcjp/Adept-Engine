#include "EditorOutliner.h"
#include "UI/CompoundWidgets/UIListBox.h"
#include "UI/Core/UIWidget.h"
#include "UI/CompoundWidgets/UITab.h"
#include "UI/CompoundWidgets/UIPanel.h"
#include "UI/UIManager.h"
#include "Core/GameObject.h"

EditorOutliner::EditorOutliner()
	:UITab()
{
	List = new UIListBox(0, 0, 0, 0);
	TabPanelArea->AddChild(List);
	SetName("Outliner");
	AlwaysShowTabButton = true;
}


EditorOutliner::~EditorOutliner()
{}

void EditorOutliner::SetGameObjects(std::vector<GameObject*>& Objects)
{
#if WITH_EDITOR
	List->Clear();	
	using std::placeholders::_1;
	List->SelectionChanged = std::bind(&UIManager::SelectedCallback, _1);
	List->ItemSize = 15;
	for (int i = 0; i < Objects.size(); i++)
	{
		Objects[i]->PostChangeProperties();
		List->AddItem(Objects[i]->GetName().c_str());
	}
#endif
}

void EditorOutliner::UpdateScaled()
{
	List->SetRootSpaceSize(TabPanelArea->GetTransfrom()->GetSizeRootSpace().x, TabPanelArea->GetTransfrom()->GetSizeRootSpace().y, 0, 0);
	UITab::UpdateScaled();
}

void EditorOutliner::ReCreate()
{

}
