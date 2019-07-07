#include "ForwardRenderNode.h"
#include "Core/Assets/Scene.h"
#include "Core/BaseWindow.h"
#include "Rendering/Core/LightCulling/LightCullingEngine.h"
#include "Rendering/Core/Material.h"
#include "Rendering/Renderers/RenderEngine.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/RenderNodes/StoreNodes/ShadowAtlasStorageNode.h"
#include "Rendering/Shaders/Generation/Shader_EnvMap.h"
#include "Rendering/Shaders/Shader_Skybox.h"

ForwardRenderNode::ForwardRenderNode()
{
	OnNodeSettingChange();
}

ForwardRenderNode::~ForwardRenderNode()
{
	SafeRHIRelease(CommandList);
}

void ForwardRenderNode::OnExecute()
{
	FrameBuffer* TargetBuffer = GetFrameBufferFromInput(0);
	CommandList->ResetList();
	CommandList->StartTimer(EGPUTIMERS::MainPass);
	Scene* MainScene = GetSceneDataFromInput(1);
	ensure(MainScene);
	UsePreZPass = (GetInput(0)->GetStoreTarget()->DataFormat == StorageFormats::PreZData);

	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(Material::GetDefaultMaterialShader(), TargetBuffer);
	desc.RenderPassDesc = RHIRenderPassDesc(TargetBuffer, UsePreZPass ? ERenderPassLoadOp::Load : ERenderPassLoadOp::Clear);

	CommandList->SetPipelineStateDesc(desc);

	if (GetInput(2)->IsValid())
	{
		GetShadowDataFromInput(2)->BindPointArray(CommandList, MainShaderRSBinds::PointShadow);
	}


	CommandList->BeginRenderPass(desc.RenderPassDesc);

	glm::ivec2 Res = glm::ivec2(TargetBuffer->GetWidth(), TargetBuffer->GetHeight());
	CommandList->SetRootConstant(MainShaderRSBinds::ResolutionCBV, 2, &Res, 0);

	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
		//CommandList->SetFrameBufferTexture(SceneRender->probes[0]->CapturedTexture, MainShaderRSBinds::SpecBlurMap);
		CommandList->SetTexture(MainScene->GetLightingData()->SkyBox, MainShaderRSBinds::SpecBlurMap);
	}

#if 0
	CommandList->SetTexture(MainScene->GetLightingData()->SkyBox, MainShaderRSBinds::DiffuseIr);
	CommandList->SetTexture(Defaults::GetDefaultTexture(), MainShaderRSBinds::EnvBRDF);
#else
	if (BaseWindow::GetCurrentRenderer()->DDOs[CommandList->GetDeviceIndex()].ConvShader != nullptr)
	{
		CommandList->SetFrameBufferTexture(BaseWindow::GetCurrentRenderer()->DDOs[CommandList->GetDeviceIndex()].ConvShader->CubeBuffer, MainShaderRSBinds::DiffuseIr);
		CommandList->SetFrameBufferTexture(BaseWindow::GetCurrentRenderer()->DDOs[CommandList->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, MainShaderRSBinds::EnvBRDF);
	}
#endif

	SceneRenderer::Get()->GetLightCullingEngine()->BindLightBuffer(CommandList);
	SceneRenderer::Get()->SetupBindsForForwardPass(CommandList, 0);
	MeshPassRenderArgs Args;
	Args.PassType = ERenderPass::BasePass;
	Args.UseDeferredShaders = false;
	SceneRenderer::Get()->MeshController->RenderPass(Args, CommandList);

	CommandList->EndRenderPass();
	Shader_Skybox* SkyboxShader = ShaderComplier::GetShader<Shader_Skybox>();
	SkyboxShader->Render(SceneRenderer::Get(), CommandList, TargetBuffer, nullptr);
	CommandList->EndTimer(EGPUTIMERS::MainPass);
	TargetBuffer->MakeReadyForComputeUse(CommandList);
	CommandList->Execute();
	PassNodeThough(0, StorageFormats::LitScene);
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
	AddInput(EStorageType::ShadowData, StorageFormats::ShadowData);
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene, "Lit output");
	//if (UseLightCulling)
	{
		AddInput(EStorageType::Buffer, StorageFormats::LightCullingData, "Light culling data");
	}
	//if (UsePreZPass)
	{
		//AddInput(EStorageType::Framebuffer, StorageFormats::PreZData, "Pre-Z data");
	}

}
