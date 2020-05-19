#pragma once
#include "UI/CompoundWidgets/UITab.h"

class UIListBox;
class BaseAsset;
class UIAssetPickerTab :public UITab
{
public:
	UIAssetPickerTab();
	~UIAssetPickerTab();

	void Setup(ClassTypeID Type, std::function<void(BaseAsset*)> AssetPickCallback);

	static void CreateAssetPicker(ClassTypeID id, std::function<void(BaseAsset*)> AssetPickCallback);
private:
	std::function<void(BaseAsset*)> m_AssetPickCallback;
	UIListBox* List = nullptr;
	ClassTypeID TypeTarget = 0;
	std::vector<BaseAsset*> ListedAssets;
	static UIAssetPickerTab* Instance;
	static UIWindow* WindowInstance;
};

