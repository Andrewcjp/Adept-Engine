
#include "UIAssetManager.h"
#include "UI/CompoundWidgets/UIListBox.h"
#include "UI/UIManager.h"
UIAssetManager::UIAssetManager() : UITab()
{
	//leftpanel = new UIListBox(1, 1, 1, 1);
	SetName("Project");
}

UIAssetManager::~UIAssetManager()
{}

//void UIAssetManager::ResizeView(int w, int h, int x, int y)
//{
//	UIBox::ResizeView(w, h, x, y);
//	leftpanel->ResizeView(UIManager::GetScaledWidth(LeftSideWidth), h, x, y);
//}

