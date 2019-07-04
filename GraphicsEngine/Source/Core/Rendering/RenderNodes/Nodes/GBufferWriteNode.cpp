#include "GBufferWriteNode.h"
#include "Rendering/Core/Material.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHICommandList.h"
#include "RHI/RHITimeManager.h"


GBufferWriteNode::GBufferWriteNode()
{
	ViewMode = EViewMode::PerView;
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat);
	AddOutput(EStorageType::Framebuffer, StorageFormats::GBufferData);
}

GBufferWriteNode::~GBufferWriteNode()
{}

void GBufferWriteNode::OnExecute()
{
	CommandList->ResetList();
	CommandList->StartTimer(EGPUTIMERS::DeferredWrite);
	ensure(GetInput(0)->GetStoreTarget());
	FrameBuffer* GBuffer = GetFrameBufferFromInput(0);
	
	RHIPipeLineStateDesc desc;
	desc.DepthStencilState.DepthWrite = true;
	desc.ShaderInUse = Material::GetDefaultMaterialShader();
	desc.FrameBufferTarget = GBuffer;
	CommandList->SetPipelineStateDesc(desc);

	SceneRenderer::Get()->SetupBindsForForwardPass(CommandList, 0);
	//CommandList->SetRenderTarget(gbuffer);
	//CommandList->ClearFrameBuffer(gbuffer);
	RHIRenderPassDesc D = RHIRenderPassDesc(GBuffer, ERenderPassLoadOp::Clear);
	D.FinalState = GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	CommandList->BeginRenderPass(D);
	MeshPassRenderArgs Args;
	Args.PassType = ERenderPass::BasePass;
	Args.UseDeferredShaders = true;
	SceneRenderer::Get()->MeshController->RenderPass(Args, CommandList);
	//SceneRender->RenderScene(CommandList, false, gbuffer, false, eyeindex);



	CommandList->EndRenderPass();
	GBuffer->MakeReadyForComputeUse(CommandList);
	CommandList->EndTimer(EGPUTIMERS::DeferredWrite);
	CommandList->Execute();
	GetInput(0)->GetStoreTarget()->DataFormat = StorageFormats::GBufferData;
	//pass the input to the output now changed
	GetOutput(0)->SetStore(GetInput(0)->GetStoreTarget());
}

void GBufferWriteNode::OnSetupNode()
{
	CommandList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
