#include "PostProcessNode.h"
#include "Rendering/PostProcessing/PostProcessing.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"

PostProcessNode::PostProcessNode()
{
	OnNodeSettingChange();
	NodeEngineType = ENodeQueueType::Compute;
	ViewMode = EViewMode::PerView;
	//SetNodeActive(false);
	PostProcessing::StartUp();
}



PostProcessNode::~PostProcessNode()
{}

void PostProcessNode::OnExecute()
{
	PostProcessing::Get()->ExecPPStack(GetFrameBufferFromInput(0));
	PassNodeThough(0);
}

void PostProcessNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::LitScene);
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene);
}
