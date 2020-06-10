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
#include "../../Renderers/Terrain/TerrainRenderer.h"
#include "../../Shaders/Shader_Pair.h"
#include "../../Core/Screen.h"
#include "RHI/RHITexture.h"
#include "RHI/Streaming/SamplerFeedbackEngine.h"
REGISTER_SHADER_PS(EdgeDetect, "VRX/VRX_EdgeDetect_PS");

GBufferWriteNode::GBufferWriteNode()
{
	ViewMode = EViewMode::PerView;
	AddResourceInput(EStorageType::Framebuffer, EResourceState::RenderTarget, StorageFormats::DefaultFormat);
	NodeLink* link = AddResourceInput(EStorageType::Framebuffer, EResourceState::Non_PixelShader, StorageFormats::DefaultFormat);
	link->SetOptional();
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
	ensure(GetInput(0)->GetStoreTarget());
	FrameBuffer* GBuffer = GetFrameBufferFromInput(0);
	bool	UsePreZPass = (GetInput(0)->GetStoreTarget()->DataFormat == StorageFormats::PreZData);
	RHIPipeLineStateDesc desc;
	desc.DepthStencilState.DepthWrite = !UsePreZPass;
	desc.ShaderInUse = Material::GetDefaultMaterialShader();
	desc.RenderTargetDesc = GBuffer->GetPiplineRenderDesc();
	CommandList->SetPipelineStateDesc(desc);

	SceneRenderer::Get()->SetupBindsForForwardPass(CommandList, GetEye(), GBuffer);
	RHIRenderPassDesc D = RHIRenderPassDesc(GBuffer, UsePreZPass ? ERenderPassLoadOp::Load : ERenderPassLoadOp::Clear);
	D.FinalState = EResourceState::PixelShader;
	CommandList->BeginRenderPass(D);
	MeshPassRenderArgs Args;
	Args.PassType = ERenderPass::BasePass;
	Args.UseDeferredShaders = true;
	Args.ReadDepth = UsePreZPass;
	SceneRenderer::Get()->MeshController->RenderPass(Args, CommandList);
	CommandList->EndRenderPass();
	SamplerFeedbackEngine::Get()->RenderTest(CommandList, GBuffer);
	//TerrainRenderer::Get()->RenderTerrainGBuffer(GBuffer, CommandList, GBuffer);
#if 1
	if (GetInput(1)->IsValid())
	{
		if (Test == nullptr)
		{
			Test = new Shader_Pair(RHI::GetDefaultDevice(), { "Deferred_LightingPass_vs","VRX/VRX_EdgeDetect_PS" }, { EShaderType::SHADER_VERTEX,EShaderType::SHADER_FRAGMENT });
		}
		FrameBuffer* TargetBuffer = GetFrameBufferFromInput(1);
		GBuffer->SetResourceState(CommandList, EResourceState::PixelShader);
		TargetBuffer->SetResourceState(CommandList, EResourceState::RenderTarget);
		desc = RHIPipeLineStateDesc::CreateDefault(Test, GetFrameBufferFromInput(1));
		desc.DepthStencilState.DepthEnable = false;
		desc.InitOLD(false, false, false);
		CommandList->SetPipelineStateDesc(desc);
		CommandList->BeginRenderPass(RHIRenderPassDesc(TargetBuffer, ERenderPassLoadOp::Clear));
		CommandList->SetFrameBufferTexture(GBuffer, "GBuffer_Normal", 1);
		glm::ivec2 Resoloution = Screen::GetScaledRes();
		CommandList->SetRootConstant("ResData", 2, &Resoloution);
		GBuffer->GetDepthStencil()->SetState(CommandList, EResourceState::PixelShader);
		CommandList->SetTexture2(GBuffer->GetDepthStencil(), "GBuffer_Depth");
		//	CommandList->SetFrameBufferTexture(GBuffer, "GBuffer_Depth", 2);
		SceneRenderer::DrawScreenQuad(CommandList);
		CommandList->EndRenderPass();
		TargetBuffer->SetResourceState(CommandList, EResourceState::Non_PixelShader);
		GBuffer->GetDepthStencil()->SetState(CommandList, EResourceState::RenderTarget);
	}
#endif
	SetEndStates(CommandList);
	CommandList->Execute();

}

void GBufferWriteNode::OnSetupNode()
{
	CommandList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
