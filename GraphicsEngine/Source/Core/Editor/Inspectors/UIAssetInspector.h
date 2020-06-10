#pragma once
#include "Editor/UIInspectorBase.h"
#if WITH_EDITOR
class BaseAsset;
class UIAssetInspector : public UIInspectorBase
{
public:
	UIAssetInspector();
	static UIAssetInspector* Get() { return Instance; }
	void SetTarget(BaseAsset* a);
protected:
	void OnPopulateFields() override;
	void OnInspectorFinishCreate(UIWidget* TabArea) override;
	static UIAssetInspector* Instance;
	BaseAsset* m_target = nullptr;
};

#endif