#include "GBufferWriteNode.h"
#include "Rendering/Core/Material.h"
#include "Core/Performance/PerfManager.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHICommandList.h"
#include "RHI/RHITimeManager.h"


GBufferWriteNode::GBufferWriteNode()
{
	ViewMode = EViewMode::PerView;
	AddResourceInput(EStorageType::Framebuffer, EResourceState::RenderTarget, StorageFormats::DefaultFormat);
	AddResourceOutput(EStorageType::Framebuffer, EResourceState::RenderTarget, StorageFormats::GBufferData);
	GetOutput(0)->SetLink(GetInput(0));
}

GBufferWriteNode::~GBufferWriteNode()
{
	SafeRHIRelease(CommandList);
}

void GBufferWriteNode::OnExecute()
{
	GetInput(0)->GetStoreTarget()->DataFormat = StorageFormats::GBufferData;
	//pass the input to the output now changed
	
	SCOPE_CYCLE_COUNTER_GROUP("GBufferWrite", "Render");
	CommandList->ResetList();
	SetBeginStates(CommandList);
	CommandList->StartTimer(EGPUTIMERS::DeferredWrite);
	ensure(GetInput(0)->GetStoreTarget());
	FrameBuffer* GBuffer = GetFrameBufferFromInput(0);
	bool	UsePreZPass = (GetInput(0)->GetStoreTarget()->DataFormat == StorageFormats::PreZData);
	RHIPipeLineStateDesc desc;
	desc.DepthStencilState.DepthWrite = !UsePreZPass;
	desc.ShaderInUse = Material::GetDefaultMaterialShader();
	desc.RenderTargetDesc = GBuffer->GetPiplineRenderDesc();
	CommandList->SetPipelineStateDesc(desc);

	SceneRenderer::Get()->SetupBindsForForwardPass(CommandList, GetEye(),GBuffer);
	RHIRenderPassDesc D = RHIRenderPassDesc(GBuffer, UsePreZPass ? ERenderPassLoadOp::Load : ERenderPassLoadOp::Clear);
	D.FinalState = GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	CommandList->BeginRenderPass(D);
	MeshPassRenderArgs Args;
	Args.PassType = ERenderPass::BasePass;
	Args.UseDeferredShaders = true;
	Args.ReadDepth = UsePreZPass;
	SceneRenderer::Get()->MeshController->RenderPass(Args, CommandList);
	CommandList->EndRenderPass();

	SetEndStates(CommandList);
	CommandList->EndTimer(EGPUTIMERS::DeferredWrite);
	CommandList->Execute();

}

void GBufferWriteNode::OnSetupNode()
{
	CommandList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
