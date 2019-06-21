#include "OutputToScreenNode.h"
#include "../StorageNodeFormats.h"
#include "../../Core/RenderingUtils.h"
#include "Core/Assets/ShaderComplier.h"
#include "../../Shaders/PostProcess/Shader_Compost.h"


OutputToScreenNode::OutputToScreenNode()
{
	OnNodeSettingChange();
}


OutputToScreenNode::~OutputToScreenNode()
{}

void OutputToScreenNode::OnExecute()
{
	FrameBuffer* Target = GetFrameBufferFromInput(0);
	ScreenWriteList->ResetList();
	ScreenWriteList->BeginRenderPass(RHI::GetRenderPassDescForSwapChain(true));

	RHIPipeLineStateDesc D = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_Compost>());
	D.Cull = false;
	ScreenWriteList->SetPipelineStateDesc(D);
	ScreenWriteList->SetFrameBufferTexture(Target, 0);

	RenderingUtils::RenderScreenQuad(ScreenWriteList);
	ScreenWriteList->EndRenderPass();
	ScreenWriteList->Execute();
}

void OutputToScreenNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat, "Frame ");
}

void OutputToScreenNode::OnSetupNode()
{
	ScreenWriteList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
