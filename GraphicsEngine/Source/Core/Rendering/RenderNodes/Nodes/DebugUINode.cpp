#include "DebugUINode.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHICommandList.h"

DebugUINode::DebugUINode()
{
	OnNodeSettingChange();
	//SetNodeActive(false);
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
	if (CommandList->GetDeviceIndex() == 0)
	{
		DebugLineDrawer::Get()->RenderLines(FB, CommandList, GetEye());
		//DebugLineDrawer::Get2()->RenderLines(FB, CommandList, GetEye());
	}
	SetEndStates(CommandList);
	Context->GetListPool()->Flush();
	PassNodeThough(0);
}

std::string DebugUINode::GetName() const
{
	return "Debug UI";
}

void DebugUINode::RefreshNode()
{
	SetNodeActive(DebugLineDrawer::Get()->HasWork() || DebugLineDrawer::Get2()->HasWork());
	if (RHI::GetRenderSettings()->SelectedGraph == EBuiltinRenderGraphs::TEST_MGPU)
	{
		SetNodeActive(true);
	}
}

void DebugUINode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::RenderTarget, StorageFormats::LitScene, "Data");
}

void DebugUINode::OnSetupNode()
{

}

