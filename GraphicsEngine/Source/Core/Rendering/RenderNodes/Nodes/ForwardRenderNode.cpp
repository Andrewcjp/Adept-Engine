#include "ForwardRenderNode.h"
#include "Core/Assets/Scene.h"
#include "Rendering/Core/Defaults.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/Material.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/RenderNodes/StoreNodes/FrameBufferStorageNode.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHICommandList.h"
#include "Core/BaseWindow.h"
#include "../../Renderers/RenderEngine.h"
#include "../../Core/LightCulling/LightCullingEngine.h"

ForwardRenderNode::ForwardRenderNode()
{
	OnNodeSettingChange();
}

ForwardRenderNode::~ForwardRenderNode()
{}

void ForwardRenderNode::OnExecute()
{
	FrameBuffer* TargetBuffer = GetFrameBufferFromInput(0);
	CommandList->ResetList();
	CommandList->StartTimer(EGPUTIMERS::MainPass);
	Scene* MainScene = GetSceneDataFromInput(1);
	ensure(MainScene);
#if 1
	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(Material::GetDefaultMaterialShader(), TargetBuffer);
	desc.RenderPassDesc = RHIRenderPassDesc(TargetBuffer, UsePreZPass ? ERenderPassLoadOp::Load : ERenderPassLoadOp::Clear);
	//desc.RenderPassDesc.FinalState = GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	CommandList->SetPipelineStateDesc(desc);

	//#if !BASIC_RENDER_ONLY
	//	if (mShadowRenderer != nullptr)
	//	{
	//		mShadowRenderer->BindShadowMapsToTextures(CommandList);
	//	}
	//#endif
	CommandList->BeginRenderPass(desc.RenderPassDesc);
	//if (RHI::GetMGPUSettings()->SplitShadowWork || RHI::GetMGPUSettings()->SFRSplitShadows)
	{
		glm::ivec2 Res = glm::ivec2(TargetBuffer->GetWidth(), TargetBuffer->GetHeight());
		CommandList->SetRootConstant(MainShaderRSBinds::ResolutionCBV, 2, &Res, 0);
	}
	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
		//CommandList->SetFrameBufferTexture(SceneRender->probes[0]->CapturedTexture, MainShaderRSBinds::SpecBlurMap);
		CommandList->SetTexture(MainScene->GetLightingData()->SkyBox, MainShaderRSBinds::SpecBlurMap);
	}
	//if (DDOs[CommandList->GetDeviceIndex()].ConvShader != nullptr)
	//{
	//	CommandList->SetFrameBufferTexture(DDOs[CommandList->GetDeviceIndex()].ConvShader->CubeBuffer, MainShaderRSBinds::DiffuseIr);
	//	CommandList->SetFrameBufferTexture(DDOs[CommandList->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, MainShaderRSBinds::EnvBRDF);
	//}
#if 1
	CommandList->SetTexture(MainScene->GetLightingData()->SkyBox, MainShaderRSBinds::DiffuseIr);
	CommandList->SetTexture(Defaults::GetDefaultTexture(), MainShaderRSBinds::EnvBRDF);
#endif
	BaseWindow::GetCurrentRenderer()->LightCulling->BindLightBuffer(CommandList);
	SceneRenderer::Get()->SetupBindsForForwardPass(CommandList, 0);
	//SceneRenderer::Get()->RenderScene(CommandList, false, TargetBuffer, false, 0);
	MeshPassRenderArgs Args;
	Args.PassType = ERenderPass::BasePass;
	Args.UseDeferredShaders = false;
	SceneRenderer::Get()->Controller->RenderPass(Args, CommandList);
	//CommandList->SetRenderTarget(nullptr);
//#if !BASIC_RENDER_ONLY
//	mShadowRenderer->Unbind(CommandList);
//#endif
	//LightCulling->Unbind(CommandList);
	CommandList->EndRenderPass();
#endif
	CommandList->EndTimer(EGPUTIMERS::MainPass);
	TargetBuffer->MakeReadyForComputeUse(CommandList);
	CommandList->Execute();
	GetOutput(0)->SetStore(GetInput(0)->GetStoreTarget());
}

void ForwardRenderNode::OnSetupNode()
{
	CommandList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}

std::string ForwardRenderNode::GetName() const
{
	return "Forward Pass";
}

void ForwardRenderNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat, "Output buffer");
	AddInput(EStorageType::SceneData, StorageFormats::DefaultFormat, "Scene Data");
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene, "Lit output");
	if (UseLightCulling)
	{
		AddInput(EStorageType::Buffer, StorageFormats::LightCullingData, "Light culling data");
	}
	if (UsePreZPass)
	{
		AddInput(EStorageType::Framebuffer, StorageFormats::PreZData, "Pre-Z data");
	}
}
