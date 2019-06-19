#include "Stdafx.h"
#include "DeferredLightingNode.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StorageNode.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"

DeferredLightingNode::DeferredLightingNode()
{
	ViewMode = EViewMode::PerView;
	Inputs.push_back(new NodeLink(EStorageType::Framebuffer, StorageFormats::GBufferData));
	Outputs.push_back(new NodeLink(EStorageType::Framebuffer, StorageFormats::DefaultFormat));
}


DeferredLightingNode::~DeferredLightingNode()
{}

void DeferredLightingNode::SetupNode()
{
}

void DeferredLightingNode::OnExecute()
{
	StorageNode* n = GetInput(0)->GetStoreTarget();
	ensure(n);
}
