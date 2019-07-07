#include "OutputToScreenNode.h"
#include "Core/Assets/ShaderComplier.h"
#include "Rendering/Core/RenderingUtils.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/Shaders/PostProcess/Shader_Compost.h"
#include "RHI/RHICommandList.h"
#include "RHI/DeviceContext.h"
#include "../../Core/DebugLineDrawer.h"

OutputToScreenNode::OutputToScreenNode()
{
	OnNodeSettingChange();
}


OutputToScreenNode::~OutputToScreenNode()
{
	SafeRHIRelease(ScreenWriteList);
}

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

std::string OutputToScreenNode::GetName() const
{
	return "Output To Screen";
}

void OutputToScreenNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat, "Frame ");
}

void OutputToScreenNode::OnSetupNode()
{
	ScreenWriteList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
