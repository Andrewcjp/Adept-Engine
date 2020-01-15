#include "EditorOutliner.h"
#include "UI/CompoundWidgets/UIListBox.h"
#include "UI/Core/UIWidget.h"
#include "UI/CompoundWidgets/UITab.h"
#include "UI/CompoundWidgets/UIPanel.h"
#include "UI/UIManager.h"

EditorOutliner::EditorOutliner()
	:UITab()
{
	List = new UIListBox(0, 0, 0, 0);
	TabPanelArea->AddChild(List);
	name = "Outliner";
}


EditorOutliner::~EditorOutliner()
{}

void EditorOutliner::SetGameObjects(std::vector<GameObject*>& Objects)
{
	List->RemoveAll();	
	using std::placeholders::_1;
	List->SelectionChanged = std::bind(&UIManager::SelectedCallback, _1);
	for (int i = 0; i < Objects.size(); i++)
	{
		Objects[i]->PostChangeProperties();
		List->AddItem(Objects[i]->GetName().c_str());
	}
}

void EditorOutliner::UpdateScaled()
{
	List->SetRootSpaceSize(TabPanelArea->GetTransfrom()->GetSizeRootSpace().x, TabPanelArea->GetTransfrom()->GetSizeRootSpace().y, 0, 0);
	UITab::UpdateScaled();
}

void EditorOutliner::ReCreate()
{

}
