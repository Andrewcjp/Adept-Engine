#include "Stdafx.h"
#include "ZPrePassNode.h"
#include "../StorageNodeFormats.h"
#include "Core/Assets/ShaderComplier.h"
#include "../../Core/Shader_PreZ.h"
#include "../../Core/SceneRenderer.h"


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
	Cmdlist->StartTimer(EGPUTIMERS::PreZ);
	RHIPipeLineStateDesc desc;
	desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_PreZ>(), Target);
	desc.DepthCompareFunction = COMPARISON_FUNC_LESS;
	desc.DepthStencilState.DepthWrite = true;
	desc.RenderPassDesc = RHIRenderPassDesc(Target, ERenderPassLoadOp::Clear);
	Cmdlist->SetPipelineStateDesc(desc);
	Cmdlist->BeginRenderPass(RHIRenderPassDesc(Target, ERenderPassLoadOp::Clear));

	SceneRenderer::Get()->RenderScene(Cmdlist, true, Target, false, 0);
	Cmdlist->EndRenderPass();
	Cmdlist->EndTimer(EGPUTIMERS::PreZ);
	Cmdlist->Execute();
	PassNodeThough(0, StorageFormats::PreZData);
}

std::string ZPrePassNode::GetName() const
{
	return "Z PrePass";
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
