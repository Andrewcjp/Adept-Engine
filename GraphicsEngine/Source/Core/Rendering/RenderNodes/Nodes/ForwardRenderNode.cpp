#include "ForwardRenderNode.h"
#include "Core/Assets/Scene.h"
#include "Core/BaseWindow.h"
#include "Rendering/Core/LightCulling/LightCullingEngine.h"
#include "Rendering/Core/Material.h"
#include "Rendering/Core/ReflectionEnviroment.h"
#include "Rendering/Core/SceneRenderer.h"

#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/RenderNodes/StoreNodes/ShadowAtlasStorageNode.h"
#include "Rendering/Shaders/Generation/Shader_EnvMap.h"
#include "Rendering/Shaders/Shader_Skybox.h"
#include "../../Shaders/Shader_Main.h"
#include "../../Core/FrameBuffer.h"
#include "Core/Performance/PerfManager.h"
#include "RHI/SFRController.h"
#include "RHI/DeviceContext.h"

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
	SCOPE_CYCLE_COUNTER_GROUP("ForwardRender", "Render");
	FrameBuffer* TargetBuffer = GetFrameBufferFromInput(0);
	CommandList->ResetList();	
	SetBeginStates(CommandList);
	Scene* MainScene = GetSceneDataFromInput(1);
	ensure(MainScene);
	UsePreZPass = (GetInput(0)->GetStoreTarget()->DataFormat == StorageFormats::PreZData);

	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(Material::GetDefaultMaterialShader(), TargetBuffer);
	desc.RenderPassDesc = RHIRenderPassDesc(TargetBuffer, UsePreZPass ? ERenderPassLoadOp::Load : ERenderPassLoadOp::Clear);

	CommandList->SetPipelineStateDesc(desc);

	CommandList->BeginRenderPass(desc.RenderPassDesc);
	glm::ivec2 Res = glm::ivec2(TargetBuffer->GetWidth(), TargetBuffer->GetHeight());
	if (RHI::GetRenderSettings()->GetCurrnetSFRSettings().Enabled)
	{
		CommandList->SetScissorRect(SFRController::GetScissor(CommandList->GetDeviceIndex(), Res));
	}
	if (!RHI::IsVulkan())
	{
		CommandList->SetRootConstant("Resolution", 2, &Res, 0);
	}
	SceneRenderer::Get()->GetReflectionEnviroment()->BindStaticSceneEnivoment(CommandList, false);
	//SceneRenderer::Get()->GetReflectionEnviroment()->BindDynamicReflections(CommandList, false);

	//CommandList->SetVRSShadingRate(VRS_SHADING_RATE::SHADING_RATE_2X2);
	SceneRenderer::Get()->SetupBindsForForwardPass(CommandList, GetEye(), TargetBuffer);
	SceneRenderer::Get()->GetLightCullingEngine()->BindLightBuffer(CommandList);
	MeshPassRenderArgs Args;
	Args.PassType = ERenderPass::BasePass;
	Args.UseDeferredShaders = false;
	Args.ReadDepth = UsePreZPass;
	Args.PassData = this;
	Args.Eye = GetEye();
	SceneRenderer::Get()->MeshController->RenderPass(Args, CommandList);

	CommandList->EndRenderPass();
	Shader_Skybox* SkyboxShader = ShaderCompiler::GetShader<Shader_Skybox>();
	SkyboxShader->Render(SceneRenderer::Get(), CommandList, TargetBuffer, nullptr);
	

	SetEndStates(CommandList);
	CommandList->Execute();
	PassNodeThough(0, StorageFormats::LitScene);
}

void ForwardRenderNode::BindLightingData(RHICommandList* list, ForwardRenderNode* node)
{
	SceneRenderer::Get()->GetReflectionEnviroment()->BindStaticSceneEnivoment(list, false);
	//SceneRenderer::Get()->GetReflectionEnviroment()->BindDynamicReflections(CommandList, false);

	SceneRenderer::Get()->BindLightsBuffer(list);
	SceneRenderer::Get()->GetLightCullingEngine()->BindLightBuffer(list);
	if (node->GetInput(2)->IsValid())
	{
		node->GetShadowDataFromInput(2)->BindPointArray(list, "g_Shadow_texture2");
	}
	if (node->GetInput(3)->IsValid())
	{
		list->SetFrameBufferTexture(node->GetFrameBufferFromInput(3), "PerSampledShadow");
	}
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
	AddResourceInput(EStorageType::Framebuffer, EResourceState::RenderTarget, StorageFormats::DefaultFormat, "Output buffer");
	AddInput(EStorageType::SceneData, StorageFormats::DefaultFormat, "Scene Data");
	AddInput(EStorageType::ShadowData, StorageFormats::ShadowData);
	AddResourceInput(EStorageType::Framebuffer, EResourceState::PixelShader, StorageFormats::PreSampleShadowData, "Shadow mask")->SetOptional();
	//if (UseLightCulling)
	{
		NodeLink* lnk = AddInput(EStorageType::Buffer, StorageFormats::LightCullingData, "Light culling data");
		lnk->SetOptional();
	}
	//if (UsePreZPass)
	{
		//AddInput(EStorageType::Framebuffer, StorageFormats::PreZData, "Pre-Z data");
	}
	
}
