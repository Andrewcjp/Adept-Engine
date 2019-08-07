#include "PostProcessNode.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "../../PostProcessing/PostProcessing.h"


PostProcessNode::PostProcessNode()
{
	OnNodeSettingChange();
	NodeEngineType = ENodeQueueType::Compute;
	ViewMode = EViewMode::PerView;
#if !TEST_VRR
	SetNodeActive(false);
#endif
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
