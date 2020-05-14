#include "PostProcessNode.h"
#include "Rendering/PostProcessing/PostProcessing.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "../NodeLink.h"
#include "../StoreNodes/FrameBufferStorageNode.h"
#include "RHI/RHICommandList.h"
#include "Rendering/Core/FrameBuffer.h"
#include "RHI/DeviceContext.h"

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
	if (RHI::GetRenderSettings()->GetVRXSettings().UseVRR())
	{
		AddResourceInput(EStorageType::Framebuffer, EResourceState::Non_PixelShader, StorageFormats::LitScene, "VRX Image");
	}
	
}

void PostProcessNode::OnSetupNode()
{
	CommandList = RHI::CreateCommandList(ECommandListType::Compute, Context);
}
