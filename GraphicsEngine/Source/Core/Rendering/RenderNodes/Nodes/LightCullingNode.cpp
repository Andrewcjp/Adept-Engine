#include "LightCullingNode.h"
#include "Rendering/Core/LightCulling/LightCullingEngine.h"
#include "Rendering/Shaders/GlobalShaderLibrary.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHICommandList.h"
#include "Rendering/Core/SceneRenderer.h"
#include "RHI/RHIBufferGroup.h"
#include "Rendering/Core/Screen.h"
#include "Rendering/Core/FrameBuffer.h"
#include "SimpleNode.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StoreNodes/FrameBufferStorageNode.h"
#include "RHI/RHITypes.h"
#include "Rendering/Shaders/Shader_Pair.h"

LightCullingNode::LightCullingNode()
{
	NodeEngineType = ECommandListType::Compute;
	//SetNodeActive(false);
	OnNodeSettingChange();
}


LightCullingNode::~LightCullingNode()
{}

void LightCullingNode::OnExecute()
{
	RHICommandList* List = GetListAndReset();

	/*SceneRenderer::Get()->GetLightCullingEngine()->LaunchCullingForScene(List, EEye::Left);*/
	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(GlobalShaderLibrary::LightCullingShader->Get());
	List->SetPipelineStateDesc(desc);
	SceneRenderer::Get()->GetLightCullingEngine()->UpdateLightStatsBuffer();
	List->SetConstantBufferView(SceneRenderer::Get()->GetLightCullingEngine()->LightCullBuffer->Get(List), 0, "LightBuffer");
	SceneRenderer::Get()->BindMvBuffer(List, "CameraData");

	RHIBuffer* TileList = GetBufferFromInput(0);
	FrameBuffer* DepthStencil = GetFrameBufferFromInput(1);
	List->SetUAV(TileList, "DstLightList");
	List->SetBuffer(SceneRenderer::Get()->GetLightCullingEngine()->GetLightDataBuffer()->Get(List), "LightList");
	List->SetTexture2(DepthStencil->GetDepthStencil(), "Depth");
	List->Dispatch(LightCullingEngine::GetLightGridDim().x, LightCullingEngine::GetLightGridDim().y, 1);
	List->UAVBarrier(TileList);
	SceneRenderer::Get()->GetLightCullingEngine()->LightCullingBuffer = TileList;
	ExecuteList(true);
}

std::string LightCullingNode::GetName() const
{
	return "Light Culling";
}



void ResizeBuffer(RHIBufferDesc& desc, glm::ivec2 ScreenSize)
{
	desc.AllowUnorderedAccess = true;
	desc.Stride = 4;
	desc.ElementCount = LightCullingEngine::GetLightGridDim().x * LightCullingEngine::GetLightGridDim().y * (RHI::GetRenderConstants()->MAX_LIGHTS);
	desc.Accesstype = EBufferAccessType::GPUOnly;
}

void LightCullingNode::OnNodeSettingChange()
{
	RHIBufferDesc Desc;
	ResizeBuffer(Desc, Screen::GetScaledRes());

	NodeInputStruct.CulledLightList = AddBufferResource(EResourceState::UAV, Desc, "Light Culling data", &ResizeBuffer);
	NodeInputStruct.DepthBuffer = AddResourceInput(EStorageType::Framebuffer, EResourceState::Non_PixelShader, "", "Depth Buffer");
}

void LightCullingNode::OnSetupNode()
{

}


void LightCullingNode::AddApplyToGraph(RenderGraph* Graph, StorageNode* gBuffer, StorageNode* ShadowMask, StorageNode* MainBuffer)
{

	SimpleNode* Apply = new SimpleNode("Tiled Light Apply",
		[&](SimpleNode* N)
	{
		N->SetEngineType(ECommandListType::Compute);
		Data.TileList = N->AddResourceInput(EStorageType::Buffer, EResourceState::Non_PixelShader, StorageFormats::DontCare, "Tile List");
		Data.TileList->SetStore(NodeInputStruct.CulledLightList->GetStoreTarget());
		Data.GBuffer = N->AddResourceInput(EStorageType::Framebuffer, EResourceState::Non_PixelShader, StorageFormats::DontCare, "GBuffer");
		Data.GBuffer->SetStore(gBuffer);
		Data.ShadowMask = N->AddResourceInput(EStorageType::Framebuffer, EResourceState::Non_PixelShader, StorageFormats::DontCare, "Shadow Mask");
		Data.ShadowMask->SetStore(ShadowMask);
		Data.MainBuffer = N->AddResourceInput(EStorageType::Framebuffer, EResourceState::UAV, StorageFormats::DontCare, "HDR out");
		Data.MainBuffer->SetStore(MainBuffer);
	},
		[&](RHICommandList* list)
	{
		ExecuteApply(Data, list);
	});
	Graph->AddNode(Apply);
}

void LightCullingNode::ExecuteApply(ApplyPassData& Data, RHICommandList* list)
{
	list->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(GlobalShaderLibrary::TiledLightingApplyShader->Get(list)));
	FrameBuffer* GBuffer = Data.GBuffer->GetTarget<FrameBufferStorageNode>()->GetFramebuffer();
	list->SetFrameBufferTexture(GBuffer, "PosTexture");
	list->SetFrameBufferTexture(GBuffer, "NormalTexture", 1);
	list->SetFrameBufferTexture(GBuffer, "AlbedoTexture", 2);
	list->SetFrameBufferTexture(Data.ShadowMask->GetTarget<FrameBufferStorageNode>()->GetFramebuffer(), "PerSampledShadow");
	FrameBuffer* HDROut = Data.MainBuffer->GetTarget<FrameBufferStorageNode>()->GetFramebuffer();
	list->SetUAV(HDROut, "OutBuffer");
	list->SetConstantBufferView(SceneRenderer::Get()->GetLightCullingEngine()->LightCullBuffer->Get(list), 0, "LightBuffer");
	list->SetBuffer(SceneRenderer::Get()->GetLightCullingEngine()->GetLightDataBuffer()->Get(list), "LightList");
	list->SetBuffer(Data.TileList->GetTarget<BufferStorageNode>()->GetBuffer(), "LightIndexs");
	SceneRenderer::Get()->BindMvBuffer(list, "SceneConstantBuffer");
	list->DispatchSized(HDROut->GetWidth(), HDROut->GetHeight(), 1);
	list->UAVBarrier(HDROut);
}