#include "SubmitToHMDNode.h"
#include "..\StorageNodeFormats.h"
#include "Flow\VRBranchNode.h"

SubmitToHMDNode::SubmitToHMDNode()
{
	OnNodeSettingChange();
	NodeEngineType = ECommandListType::Compute;
}

SubmitToHMDNode::~SubmitToHMDNode()
{}

void SubmitToHMDNode::OnExecute()
{
	HasRunBegin = true;
	HasRunEnd = true;
	if (RHI::GetRenderSettings()->VRHMDMode == EVRHMDMode::SteamVR)
	{
		RHI::SubmitToVRComposter(GetFrameBufferFromInput(0), VRBranchContext->GetCurrentEye());
	}
}

bool SubmitToHMDNode::IsNodeSupported(const RenderSettings& settings)
{
	return settings.VRHMDMode != EVRHMDMode::Disabled;
}

void SubmitToHMDNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer,EResourceState::PixelShader, StorageFormats::LitScene, "both Eye Framebuffer");
}

void SubmitToHMDNode::OnValidateNode(RenderGraph::ValidateArgs & args)
{
	ValidateNode(args);
	if (!args.TargetGraph->IsVRGraph)
	{
		args.AddError("SubmitToHMDNode only valid in VR graphs");
	}
}
