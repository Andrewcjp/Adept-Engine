#include "UIAssetPickerTab.h"
#include "UI/CompoundWidgets/UIWindow.h"
#include "UI/UIManager.h"
#include "UI/CompoundWidgets/UIListBox.h"
#include "Core/Assets/Asset types/BaseAsset.h"
#include "Core/Assets/AssetDatabase.h"
#include "UI/Core/UIWidget.h"
#include "UI/CompoundWidgets/UIPanel.h"
UIAssetPickerTab* UIAssetPickerTab::Instance = nullptr;

UIWindow* UIAssetPickerTab::WindowInstance;

UIAssetPickerTab::UIAssetPickerTab()
{}

UIAssetPickerTab::~UIAssetPickerTab()
{
	SafeDelete(List);
}

void UIAssetPickerTab::Setup(ClassTypeID Type, std::function<void(BaseAsset*)> AssetPickCallback)
{
	TypeTarget = Type;
	m_AssetPickCallback = AssetPickCallback;
	//ListItems = true;

	glm::ivec2 size = TabPanelArea->GetTransfrom()->GetSizeRootSpace();
	if (List == nullptr)
	{
		List = new UIListBox(size.x, size.y, 0, 0);
		List->SetRootSpaceSize(size.x, size.y, 0, 0);
		List->GetTransfrom()->SetStretchMode(EAxisStretch::ALL);
		TabPanelArea->AddChild(List);
	}
	List->RemoveAll();
	ListedAssets.clear();
	AssetDatabase::Get()->GetAllAssetsOfType(TypeTarget, ListedAssets);
	for (int i = 0; i < ListedAssets.size(); i++)
	{
		List->AddItem(ListedAssets[i]->GetName());
	}
	if (m_AssetPickCallback != nullptr)
	{
		List->SelectionChanged = [&](int i) 
		{ 
			m_AssetPickCallback(ListedAssets[i]);
			WindowInstance->Close();
			UIManager::Get()->RemoveWidget(WindowInstance);
			Instance = nullptr;
		};
	}
}

void UIAssetPickerTab::CreateAssetPicker(ClassTypeID AssetTypeID, std::function<void(BaseAsset*)> AssetPickCallback)
{
	if (Instance != nullptr)
	{
		Instance->Setup(AssetTypeID, AssetPickCallback);
		return;
	}
	UIWindow* newWindow = new UIWindow();
	WindowInstance = newWindow;
	Instance = new UIAssetPickerTab();
	Instance->Setup(AssetTypeID, AssetPickCallback);
	newWindow->AddTab(Instance);
	newWindow->SetIsFloating(true);
	newWindow->SetTitle("Picker");
	newWindow->SetCloseable(true);
	newWindow->Priority = 200;
	newWindow->SetRootSpaceScaled(200, 500, 400, 200);
	newWindow->OnCloseCallBack = [&] { Instance = nullptr; WindowInstance == nullptr; };
	UIManager::Get()->AddWidget(newWindow);
}


