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
	FLAT_COMPUTE_START(RHI::GetDeviceContext(0));
	list->ResetList();
	{
		DECALRE_SCOPEDGPUCOUNTER(list, "SSAO");
		Shader_SSAO* SSaoshader = ShaderComplier::GetShader<Shader_SSAO>();
		RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_SSAO>());
		list->SetPipelineStateDesc(desc);


		list->SetFrameBufferTexture(GBuffer, "PosTex", 0);
		list->SetFrameBufferTexture(GBuffer, "NormalTex", 1);
		//list->SetFrameBufferTexture(GBuffer, "DepthTexture", -1);
		SSaoshader->Bind(list);
		list->SetUAV(TempSSAOData, "DstTexture");
		list->Dispatch(TargetBuffer->GetWidth(), TargetBuffer->GetHeight(), 1);
		list->UAVBarrier(TempSSAOData);


		desc.ShaderInUse = ShaderComplier::GetShader<Shader_SSAO_Merge>();
		list->SetPipelineStateDesc(desc);
		list->SetUAV(TargetBuffer, 1);
		TempSSAOData->SetResourceState(list, EResourceState::Non_PixelShader);
		list->SetFrameBufferTexture(TempSSAOData, 0);
		list->Dispatch(TargetBuffer->GetWidth(), TargetBuffer->GetHeight(), 1);
		list->UAVBarrier(TargetBuffer);
	}
	list->Execute();
	FLAT_COMPUTE_END(RHI::GetDeviceContext(0));
}

void SSAONode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::ComputeUse, StorageFormats::LitScene, "Scene");
	AddResourceInput(EStorageType::Framebuffer, EResourceState::ComputeUse, StorageFormats::GBufferData, "GBuffer");
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene);

	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.AllowUnorderedAccess = true;
	AddFrameBufferResource(EResourceState::UAV, Desc, "SSAO output");
}

void SSAONode::OnSetupNode()
{
	list = RHI::CreateCommandList(ECommandListType::Compute, Context);
}
