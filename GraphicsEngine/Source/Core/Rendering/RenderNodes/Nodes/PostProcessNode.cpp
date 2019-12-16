#include "PostProcessNode.h"
#include "Rendering/PostProcessing/PostProcessing.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "../NodeLink.h"
#include "../StoreNodes/FrameBufferStorageNode.h"


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
	FLAT_COMPUTE_START(RHI::GetDeviceContext(0));
	CommandList->ResetList();
	NodeLink* VRXImage = GetInputLinkByName("VRX Image");
	if (VRXImage != nullptr && VRXImage->IsValid())
	{
		CommandList->SetVRXShadingRateImage(StorageNode::NodeCast<FrameBufferStorageNode>(VRXImage->GetStoreTarget())->GetFramebuffer()->GetRenderTexture());
	}
	PostProcessing::Get()->ExecPPStack(GetFrameBufferFromInput(0), CommandList);
	SetEndStates(CommandList);
	CommandList->Execute();
	FLAT_COMPUTE_END(RHI::GetDeviceContext(0));
	PassNodeThough(0);
}

void PostProcessNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::ComputeUse, StorageFormats::LitScene);
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene, "Post Image");
	if (RHI::GetRenderSettings()->GetVRXSettings().EnableVRR)
	{
		AddResourceInput(EStorageType::Framebuffer, EResourceState::ComputeUse, StorageFormats::LitScene, "VRX Image");
	}
}

void PostProcessNode::OnSetupNode()
{
	CommandList = RHI::CreateCommandList(ECommandListType::Compute, Context);

}
