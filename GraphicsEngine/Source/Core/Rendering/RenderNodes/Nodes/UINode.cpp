#include "UINode.h"
#include "../StorageNodeFormats.h"
#include "UI/UIManager.h"


UINode::UINode()
{
	OnNodeSettingChange();
}


UINode::~UINode()
{}

void UINode::OnExecute()
{
	RHICommandList* list = GetListAndReset();
	UIManager::Get()->RenderWidgets(list);
	ExecuteList(true);
}

void UINode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::RenderTarget, StorageFormats::DontCare, "Main RT");
}
