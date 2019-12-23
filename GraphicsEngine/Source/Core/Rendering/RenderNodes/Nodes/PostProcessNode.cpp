#include "PostProcessNode.h"
#include "Rendering/PostProcessing/PostProcessing.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "../NodeLink.h"
#include "../StoreNodes/FrameBufferStorageNode.h"


PostProcessNode::PostProcessNode()
{
	OnNodeSettingChange();
	NodeEngineType = ECommandListType::Compute;
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
	SetBeginStates(CommandList);
	NodeLink* VRXImage = GetInputLinkByName("VRX Image");
	if (VRXImage != nullptr && VRXImage->IsValid())
	{
		CommandList->SetVRXShadingRateImage(StorageNode::NodeCast<FrameBufferStorageNode>(VRXImage->GetStoreTarget())->GetFramebuffer()->GetRenderTexture());
	}
	PostProcessing::Get()->ExecPPStack(GetFrameBufferFromInput(0), CommandList);
	SetEndStates(CommandList);
	CommandList->Execute();
	FLAT_COMPUTE_END(RHI::GetDeviceContext(0));
}

void PostProcessNode::OnResourceResize()
{
	PostProcessing::Get()->Resize(nullptr);
}

void PostProcessNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::UAV, StorageFormats::LitScene);
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene, "Post Image");
	if (RHI::GetRenderSettings()->GetVRXSettings().EnableVRR)
	{
		AddResourceInput(EStorageType::Framebuffer, EResourceState::Non_PixelShader, StorageFormats::LitScene, "VRX Image");
	}
	LinkThough(0);
}

void PostProcessNode::OnSetupNode()
{
	CommandList = RHI::CreateCommandList(ECommandListType::Compute, Context);
}
