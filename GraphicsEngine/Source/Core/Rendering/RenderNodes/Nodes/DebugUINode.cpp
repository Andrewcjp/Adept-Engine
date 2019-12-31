#include "DebugUINode.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"

DebugUINode::DebugUINode()
{
	OnNodeSettingChange();
	if (RHI::IsVulkan() && RHI::GetRenderSettings()->SelectedGraph != EBuiltinRenderGraphs::Fallback)
	{
		SetNodeActive(false);
	}
}

DebugUINode::~DebugUINode()
{}

void DebugUINode::OnExecute()
{
	FrameBuffer* FB = GetFrameBufferFromInput(0);
	//debug lines are always draw to screen
	CommandList = Context->GetListPool()->GetCMDList();
	SetBeginStates(CommandList);
	if (ClearBuffer)
	{
		CommandList->ClearFrameBuffer(FB);
	}
	DebugLineDrawer::Get()->RenderLines(FB, CommandList, GetEye());
	DebugLineDrawer::Get2()->RenderLines(FB, CommandList, GetEye());	
	SetEndStates(CommandList);
	PassNodeThough(0);
}

std::string DebugUINode::GetName() const
{
	return "Debug UI";
}

void DebugUINode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::RenderTarget, StorageFormats::LitScene, "Data");
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene, "Final frame");
	LinkThough(0);
}

void DebugUINode::OnSetupNode()
{
	
}

