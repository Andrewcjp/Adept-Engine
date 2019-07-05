#include "DebugUINode.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"

DebugUINode::DebugUINode()
{
	OnNodeSettingChange();
}

DebugUINode::~DebugUINode()
{}

void DebugUINode::OnExecute()
{
	FrameBuffer* FB = GetFrameBufferFromInput(0);
	//debug lines are always draw to screen
	CommandList->ResetList();
	DebugLineDrawer::Get()->RenderLines(FB, CommandList, EEye::Left);
	DebugLineDrawer::Get2()->RenderLines(FB, CommandList, EEye::Left);
	CommandList->Execute();
	PassNodeThough(0);
}

std::string DebugUINode::GetName() const
{
	return "DebugUI";
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

