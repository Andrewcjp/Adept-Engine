#include "stdafx.h"
#include "UIAssetManager.h"
#include "UIListBox.h"
#include "UIManager.h"
UIAssetManager::UIAssetManager() :UIBox(1, 1, 1, 1)
{
	leftpanel = new UIListBox(1,1,1,1);
}


UIAssetManager::~UIAssetManager()
{
}

void UIAssetManager::ResizeView(int w, int h, int x, int y)
{
	UIBox::ResizeView(w, h, x, y);
	leftpanel->ResizeView(UIManager::GetScaledWidth(LeftSideWidth), h, x, y);
}

void UIAssetManager::MouseMove(int x, int y)
{
}

bool UIAssetManager::MouseClick(int x, int y)
{
	return false;
}
