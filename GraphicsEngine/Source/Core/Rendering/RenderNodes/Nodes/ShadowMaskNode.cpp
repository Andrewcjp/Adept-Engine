#include "ShadowMaskNode.h"
#include "Core/Assets/ShaderCompiler.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/LightCulling/LightCullingEngine.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Core/ShadowRenderer.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/RenderNodes/StoreNodes/ShadowAtlasStorageNode.h"
#include "Rendering/Shaders/MultiGPU/Shader_ShadowSample.h"
#include "Rendering/Shaders/Shader_Pair.h"
#include "RHI/RHITimeManager.h"
#include "RHI/RHICommandList.h"
#include "RHI/RHIBufferGroup.h"

ShadowMaskNode::ShadowMaskNode()
{
	OnNodeSettingChange();
	//SetNodeActive(RHI::GetRenderSettings()->GetVRXSettings().UseVRX());
	SetWorkType(ENodeWorkFocus::Compute);
}

ShadowMaskNode::~ShadowMaskNode()
{

}

void ShadowMaskNode::OnExecute()
{
	List->ResetList();
	FrameBuffer* OutputBuffer = GetFrameBufferFromInput(0);

	SetBeginStates(List);
	RHIPipeLineStateDesc psodesc = RHIPipeLineStateDesc::CreateDefault(ShaderCompiler::GetShader<Shader_ShadowSample>(Context, 4), OutputBuffer);
	psodesc.InitOLD(false, false, false);
	List->SetPipelineStateDesc(psodesc);
	int ShaderData[4] = { 0,-1,-1,-1 };
	List->SetRootConstant("PreSampleData", 4, ShaderData, 0);
	FrameBuffer* GBuffer = GetFrameBufferFromInput(2);
	List->SetFrameBufferTexture(GBuffer, "GBuffer_Pos");
	List->BeginRenderPass(RHIRenderPassDesc(OutputBuffer, ERenderPassLoadOp::Clear));

	GetShadowDataFromInput(1)->BindPointArray(List, "g_Shadow_texture2");

	List->SetBuffer(SceneRenderer::Get()->GetLightCullingEngine()->GetLightDataBuffer()->Get(List), "lights");
	SceneRenderer::DrawScreenQuad(List);
	List->EndRenderPass();
	GBuffer->SetResourceState(List, EResourceState::Non_PixelShader, true);
	//OutputBuffer->SetResourceState(List, EResourceState::Non_PixelShader);
	SetEndStates(List);
	List->Execute();

}

void ShadowMaskNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::RenderTarget, StorageFormats::PreSampleShadowData, "ShadowMask");
	AddInput(EStorageType::ShadowData, StorageFormats::ShadowData, "Shadow Maps");
	AddResourceInput(EStorageType::Framebuffer, EResourceState::PixelShader, StorageFormats::GBufferData, "GBuffer");
	
}

void ShadowMaskNode::OnSetupNode()
{
	List = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
