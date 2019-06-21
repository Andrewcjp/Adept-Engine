
#include "DeferredLightingNode.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StorageNode.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"

DeferredLightingNode::DeferredLightingNode()
{
	ViewMode = EViewMode::PerView;
	OnNodeSettingChange();
}

DeferredLightingNode::~DeferredLightingNode()
{}

void DeferredLightingNode::OnSetupNode()
{

}

void DeferredLightingNode::OnExecute()
{
	StorageNode* n = GetInput(0)->GetStoreTarget();
	ensure(n);
}

void DeferredLightingNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::GBufferData, "GBuffer");
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat, "Main buffer");
	AddOutput(EStorageType::Framebuffer, StorageFormats::DefaultFormat, "Lit scene");
}
