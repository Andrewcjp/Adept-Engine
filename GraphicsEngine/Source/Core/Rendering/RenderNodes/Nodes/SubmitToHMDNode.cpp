#include "SubmitToHMDNode.h"
#include "..\StorageNodeFormats.h"

SubmitToHMDNode::SubmitToHMDNode()
{
	OnNodeSettingChange();
}

SubmitToHMDNode::~SubmitToHMDNode()
{}

void SubmitToHMDNode::OnExecute()
{
	//
	RHI::SubmitToVRComposter(GetFrameBufferFromInput(0), EEye::Left);
}

bool SubmitToHMDNode::IsNodeSupported(const RenderSettings& settings)
{
	return settings.VRHMDMode != EVRHMDMode::Disabled;
}

void SubmitToHMDNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::LitScene, "both Eye Framebuffer");
}

void SubmitToHMDNode::OnValidateNode(RenderGraph::ValidateArgs & args)
{
	ValidateNode(args);
	if (!args.TargetGraph->IsVRGraph)
	{
		args.AddError("SubmitToHMDNode only valid in VR graphs");
	}
}
