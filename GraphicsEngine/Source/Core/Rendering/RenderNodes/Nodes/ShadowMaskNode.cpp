#include "ShadowMaskNode.h"
#include "../StorageNodeFormats.h"
#include "RHI/RHITimeManager.h"
#include "../../Shaders/Shader_Pair.h"
#include "../../Core/ShadowRenderer.h"
#include "Core/Assets/ShaderComplier.h"
#include "../../Shaders/MultiGPU/Shader_ShadowSample.h"
#include "../../Core/SceneRenderer.h"
#include "../StoreNodes/ShadowAtlasStorageNode.h"
#include "../../Core/LightCulling/LightCullingEngine.h"
#include "../NodeLink.h"
#include "../../Core/FrameBuffer.h"

ShadowMaskNode::ShadowMaskNode()
{
	OnNodeSettingChange();
}

ShadowMaskNode::~ShadowMaskNode()
{

}

void ShadowMaskNode::OnExecute()
{
	List->ResetList();
	FrameBuffer* OutputBuffer = GetFrameBufferFromInput(0);

	List->StartTimer(EGPUTIMERS::ShadowPreSample);
	SetBeginStates(List);
	RHIPipeLineStateDesc psodesc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_ShadowSample>(Context, 4), OutputBuffer);
	psodesc.InitOLD(false, false, false);
	List->SetPipelineStateDesc(psodesc);
	int ShaderData[4] = { 0,-1,-1,-1 };
	List->SetRootConstant("PreSampleData", 4, ShaderData, 0);
	FrameBuffer* GBuffer = GetFrameBufferFromInput(2);
	List->SetFrameBufferTexture(GBuffer, "GBuffer_Pos");
	List->BeginRenderPass(RHIRenderPassDesc(OutputBuffer, ERenderPassLoadOp::Clear));
	GetShadowDataFromInput(1)->BindPointArray(List, "g_Shadow_texture2");
	List->SetBuffer(SceneRenderer::Get()->GetLightCullingEngine()->GetLightDataBuffer(), "lights");
	SceneRenderer::DrawScreenQuad(List);
	List->EndRenderPass();
	OutputBuffer->SetResourceState(List, EResourceState::Non_PixelShader);
	SetEndStates(List);
	List->EndTimer(EGPUTIMERS::ShadowPreSample);
	List->Execute();
	
}

void ShadowMaskNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::RenderTarget, StorageFormats::PreSampleShadowData, "ShadowMask");
	AddInput(EStorageType::ShadowData, StorageFormats::ShadowData, "Shadow Maps");
	AddResourceInput(EStorageType::Framebuffer, EResourceState::PixelShader, StorageFormats::GBufferData, "GBuffer");
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene, "Lit scene");
	GetOutput(0)->SetLink(GetInput(0));
}

void ShadowMaskNode::OnSetupNode()
{
	List = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
