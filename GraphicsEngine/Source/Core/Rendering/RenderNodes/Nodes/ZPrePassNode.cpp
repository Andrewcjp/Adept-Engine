#include "ZPrePassNode.h"
#include "Core/Assets/ShaderComplier.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/Shaders/Shader_PreZ.h"
#include "Flow/VRBranchNode.h"
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
	//PassNodeThough(0, StorageFormats::PreZData);
	SCOPE_CYCLE_COUNTER_GROUP("ZPrePass", "Render");
	FrameBuffer* Target = GetFrameBufferFromInput(0);
	Cmdlist->ResetList();
	SetBeginStates(Cmdlist);
	Cmdlist->GetDevice()->GetTimeManager()->StartPipelineStatCapture(Cmdlist);
	Cmdlist->StartTimer(EGPUTIMERS::PreZ);
	RHIPipeLineStateDesc desc;
	desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_PreZ>(), Target);
	desc.DepthStencilState.DepthCompareFunction = COMPARISON_FUNC_LESS;
	desc.DepthStencilState.DepthWrite = true;
	desc.RenderPassDesc = RHIRenderPassDesc(Target, ERenderPassLoadOp::Clear);
	Cmdlist->SetPipelineStateDesc(desc);
	Cmdlist->BeginRenderPass(RHIRenderPassDesc(Target, ERenderPassLoadOp::Clear));

	SceneRenderer::Get()->RenderScene(Cmdlist, true, Target, false, GetEye());
	Cmdlist->EndRenderPass();
	Cmdlist->EndTimer(EGPUTIMERS::PreZ);
	Cmdlist->GetDevice()->GetTimeManager()->EndPipelineStatCapture(Cmdlist);
	SetEndStates(Cmdlist);
	Cmdlist->Execute();
	
}

void ZPrePassNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat);
	AddOutput(EStorageType::Framebuffer, StorageFormats::PreZData);
	GetOutput(0)->SetLink(GetInput(0));
}

void ZPrePassNode::OnSetupNode()
{
	Cmdlist = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
