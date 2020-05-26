
#include "UIAssetManager.h"
#include "UI/CompoundWidgets/UIListBox.h"
#include "UI/UIManager.h"
#include "UI/Core/UIWidget.h"
#include "UI/Core/UITransform.h"
#include "UI/CompoundWidgets/UIPanel.h"
#include "Core/Assets/AssetDatabase.h"
#include "Core/Assets/Asset types/BaseAsset.h"
#include "Core/Assets/Asset types/MaterialAsset.h"

UIAssetManager::UIAssetManager() : UITab()
{
	//leftpanel = new UIListBox(1, 1, 1, 1);
	SetName("Project");
	leftpanel = new UIListBox(0, 0, 0, 0);
	TabPanelArea->AddChild(leftpanel);
	Build();
}

UIAssetManager::~UIAssetManager()
{}

void UIAssetManager::UpdateScaled()
{
	UITab::UpdateScaled();
	glm::ivec2 Space = TabPanelArea->GetTransfrom()->GetSizeRootSpace();
	leftpanel->SetRootSpaceSize(200, Space.y, 0, 0);
}

void UIAssetManager::Build()
{
	leftpanel->Clear();
	Assets.clear();
	AssetDatabase::Get()->GetAllAssetsOfType(MaterialAsset::TYPEID, Assets);
	for (int i = 0; i < Assets.size(); i++)
	{
		leftpanel->AddItem(Assets[i]->GetName());
	}
}

void UIAssetManager::SelectedCallback(int i)
{

}
//void UIAssetManager::ResizeView(int w, int h, int x, int y)
//{
//	UIBox::ResizeView(w, h, x, y);
//	leftpanel->ResizeView(UIManager::GetScaledWidth(LeftSideWidth), h, x, y);
//}


