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
	RHIPipeLineStateDesc psodesc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_ShadowSample>(Context,4),OutputBuffer);
	psodesc.InitOLD(false, false, false);
	List->SetPipelineStateDesc(psodesc);
	int ShaderData[4] = { 0,1,2,3 };
	List->SetRootConstant("PreSampleData", 4, ShaderData, 0);
	FrameBuffer* GBuffer = GetFrameBufferFromInput(2);
	List->SetFrameBufferTexture(GBuffer, "GBuffer_Pos");
	List->BeginRenderPass(RHIRenderPassDesc(OutputBuffer, ERenderPassLoadOp::Clear));
	GetShadowDataFromInput(1)->BindPointArray(List, "g_Shadow_texture2");	
	List->SetBuffer(SceneRenderer::Get()->GetLightCullingEngine()->GetLightDataBuffer(),"lights");
	SceneRenderer::DrawScreenQuad(List);
	List->EndRenderPass();    
	OutputBuffer->SetResourceState(List, EResourceState::Non_PixelShader);
	List->EndTimer(EGPUTIMERS::ShadowPreSample);
	List->Execute();
	GetOutput(0)->SetStore(GetInput(0)->GetStoreTarget());
}

void ShadowMaskNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::PreSampleShadowData, "ShadowMask");
	AddInput(EStorageType::ShadowData, StorageFormats::ShadowData, "Shadow Maps");
	AddInput(EStorageType::Framebuffer, StorageFormats::GBufferData, "GBuffer");
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene, "Lit scene");
}

void ShadowMaskNode::OnSetupNode()
{
	List = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
