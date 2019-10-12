#include "ZPrePassNode.h"
#include "Core/Assets/ShaderComplier.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/Shaders/Shader_PreZ.h"
#include "Flow/VRBranchNode.h"
#include "Core/Performance/PerfManager.h"

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
	SCOPE_CYCLE_COUNTER_GROUP("ZPrePass", "Render");
	FrameBuffer* Target = GetFrameBufferFromInput(0);
	Cmdlist->ResetList();
	Cmdlist->StartTimer(EGPUTIMERS::PreZ);
	RHIPipeLineStateDesc desc;
	desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_PreZ>(), Target);
	desc.DepthCompareFunction = COMPARISON_FUNC_LESS;
	desc.DepthStencilState.DepthWrite = true;
	desc.RenderPassDesc = RHIRenderPassDesc(Target, ERenderPassLoadOp::Clear);
	Cmdlist->SetPipelineStateDesc(desc);
	Cmdlist->BeginRenderPass(RHIRenderPassDesc(Target, ERenderPassLoadOp::Clear));

	SceneRenderer::Get()->RenderScene(Cmdlist, true, Target, false, GetEye());
	Cmdlist->EndRenderPass();
	Cmdlist->EndTimer(EGPUTIMERS::PreZ);
	Cmdlist->Execute();
	PassNodeThough(0, StorageFormats::PreZData);
}

void ZPrePassNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat);
	AddOutput(EStorageType::Framebuffer, StorageFormats::PreZData);
}

void ZPrePassNode::OnSetupNode()
{
	Cmdlist = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
