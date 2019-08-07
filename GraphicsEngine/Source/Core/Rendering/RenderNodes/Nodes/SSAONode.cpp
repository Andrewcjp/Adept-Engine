#include "SSAONode.h"
#include "..\StorageNodeFormats.h"
#include "..\..\Shaders\PostProcess\Shader_SSAO.h"
#include "..\..\Core\FrameBuffer.h"
#include "RHI\RHITimeManager.h"
#include "RHI\DeviceContext.h"


SSAONode::SSAONode()
{
	OnNodeSettingChange();
	SetNodeActive(false);
}


SSAONode::~SSAONode()
{}

void SSAONode::OnExecute()
{
	FrameBuffer* TargetBuffer = GetFrameBufferFromInput(0);
	FrameBuffer* GBuffer = GetFrameBufferFromInput(1);
	FrameBuffer* TempSSAOData = GetFrameBufferFromInput(2);

	RHI::GetDeviceContext(0)->InsertGPUWait(DeviceContextQueue::Compute, DeviceContextQueue::Graphics);
	list->ResetList();
	{
		DECALRE_SCOPEDGPUCOUNTER(list, "SSAO");
		Shader_SSAO* SSaoshader = ShaderComplier::GetShader<Shader_SSAO>();
		RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_SSAO>());
		list->SetPipelineStateDesc(desc);


		list->SetFrameBufferTexture(GBuffer, "PosTex", 0);
		list->SetFrameBufferTexture(GBuffer, "NormalTex", 1);
		list->SetFrameBufferTexture(GBuffer, "DepthTexture", -1);
		SSaoshader->Bind(list);
		list->SetUAV(TempSSAOData->GetUAV(), "DstTexture");
		list->Dispatch(TargetBuffer->GetWidth(), TargetBuffer->GetHeight(), 1);
		list->UAVBarrier(TempSSAOData->GetUAV());


		desc.ShaderInUse = ShaderComplier::GetShader<Shader_SSAO_Merge>();
		list->SetPipelineStateDesc(desc);
		TargetBuffer->BindUAV(list, 1);
		list->SetFrameBufferTexture(TempSSAOData, 0);

		list->Dispatch(TargetBuffer->GetWidth(), TargetBuffer->GetHeight(), 1);
		list->UAVBarrier(TargetBuffer->GetUAV());
	}
	list->Execute();
	RHI::GetDeviceContext(0)->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
}

void SSAONode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::LitScene, "Scene");
	AddInput(EStorageType::Framebuffer, StorageFormats::GBufferData, "GBuffer");
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat, "SSAO intermediate buffer");
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene);
}

void SSAONode::OnSetupNode()
{
	list = RHI::CreateCommandList(ECommandListType::Compute, Context);
}
