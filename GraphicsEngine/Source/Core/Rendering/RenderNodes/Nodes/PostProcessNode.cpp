#include "PostProcessNode.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "../../PostProcessing/PostProcessing.h"
#include "../../Renderers/RenderEngine.h"


PostProcessNode::PostProcessNode()
{
	OnNodeSettingChange();
	NodeEngineType = ENodeQueueType::Compute;
	ViewMode = EViewMode::PerView;
	SetNodeActive(false);
}



PostProcessNode::~PostProcessNode()
{}

void PostProcessNode::OnExecute()
{
	DeviceDependentObjects d;
	d.MainFrameBuffer = GetFrameBufferFromInput(0);
	PostProcessing::Get()->ExecPPStack(&d);
	PassNodeThough(0);
}

std::string PostProcessNode::GetName() const
{
	return "Post Process";
}

void PostProcessNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::LitScene);
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene);
}
