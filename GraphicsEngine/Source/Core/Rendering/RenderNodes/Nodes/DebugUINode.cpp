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
	CommandList->ResetList();
	if (ClearBuffer)
	{
		CommandList->ClearFrameBuffer(FB);
	}
	DebugLineDrawer::Get()->RenderLines(FB, CommandList, GetEye());
	DebugLineDrawer::Get2()->RenderLines(FB, CommandList, GetEye());
	CommandList->Execute();
	PassNodeThough(0);
}

std::string DebugUINode::GetName() const
{
	return "Debug UI";
}

void DebugUINode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::LitScene, "Data");
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene, "Final frame");
}

void DebugUINode::OnSetupNode()
{
	CommandList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}

