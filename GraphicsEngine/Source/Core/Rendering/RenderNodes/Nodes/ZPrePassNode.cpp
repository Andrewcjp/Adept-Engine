#include "ZPrePassNode.h"
#include "Core/Assets/ShaderCompiler.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/Shaders/Shader_PreZ.h"

#include "Core/Performance/PerfManager.h"
#include "../NodeLink.h"
#include "RHI/RHITimeManager.h"
#include "RHI/DeviceContext.h"

ZPrePassNode::ZPrePassNode()
{
	OnNodeSettingChange();
}


ZPrePassNode::~ZPrePassNode()
{
	SafeRHIRelease(Cmdlist);
}
 
void ZPrePassNode::OnExecute()
{
	FrameBuffer* Target = GetFrameBufferFromInput(0);
	Cmdlist->ResetList();
	SetBeginStates(Cmdlist);
	Cmdlist->GetDevice()->GetTimeManager()->StartPipelineStatCapture(Cmdlist);
	RHIPipeLineStateDesc desc;
	desc = RHIPipeLineStateDesc::CreateDefault(ShaderCompiler::GetShader<Shader_PreZ>(), Target);
	desc.DepthStencilState.DepthCompareFunction = COMPARISON_FUNC_LESS;
	desc.DepthStencilState.DepthWrite = true;
	desc.RenderPassDesc = RHIRenderPassDesc(Target, ERenderPassLoadOp::Clear);
	Cmdlist->SetPipelineStateDesc(desc);
	Cmdlist->BeginRenderPass(RHIRenderPassDesc(Target, ERenderPassLoadOp::Clear));

	SceneRenderer::Get()->RenderScene(Cmdlist, true, Target, false, GetEye());
	Cmdlist->EndRenderPass();
	Cmdlist->GetDevice()->GetTimeManager()->EndPipelineStatCapture(Cmdlist);
	SetEndStates(Cmdlist);
	Cmdlist->Execute();

}

void ZPrePassNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::RenderTarget, StorageFormats::DefaultFormat);
}

void ZPrePassNode::OnSetupNode()
{
	Cmdlist = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
