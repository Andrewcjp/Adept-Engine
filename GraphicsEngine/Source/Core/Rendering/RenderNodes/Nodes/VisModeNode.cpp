#include "VisModeNode.h"
#include "Core/Assets/ShaderComplier.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/RenderingUtils.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/Shaders/PostProcess/Shader_DebugOutput.h"

VisModeNode::VisModeNode()
{
	OnNodeSettingChange();
}

VisModeNode::~VisModeNode()
{}

void VisModeNode::OnExecute()
{
	PassNodeThough(0);
	ERenderDebugOutput::Type mode = RHI::GetRenderSettings()->GetDebugRenderMode();
	if (mode == ERenderDebugOutput::Off)
	{
		return;
	}
	DebugList->ResetList();
	switch (mode)
	{
		case ERenderDebugOutput::Off:
			break;
		case ERenderDebugOutput::GBuffer_Pos:
		case ERenderDebugOutput::GBuffer_Normal:
		case ERenderDebugOutput::GBuffer_Material:
		case ERenderDebugOutput::GBuffer_RoughNess:
		case ERenderDebugOutput::GBuffer_Metallic:
			RenderGBufferModes(mode);
			break;
		case ERenderDebugOutput::Scene_Metallic:
		case ERenderDebugOutput::Scene_RoughNess:
		case ERenderDebugOutput::Scene_LightRange:
		case ERenderDebugOutput::Scene_UVs:
			RenderForwardMode(mode);
			break;
	}


	DebugList->Execute();

}

std::string VisModeNode::GetName() const
{
	return "View Modes";
}

void VisModeNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::LitScene);
	AddInput(EStorageType::Framebuffer, StorageFormats::GBufferData);
	AddInput(EStorageType::SceneData, StorageFormats::DefaultFormat);

	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene);
}

void VisModeNode::OnSetupNode()
{
	DebugList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}

void VisModeNode::RenderGBufferModes(ERenderDebugOutput::Type currentDebugType)
{
	if (!GetInput(1)->IsValid())
	{
		return;
	}
	FrameBuffer* FB = GetFrameBufferFromInput(0);
	FrameBuffer* gBuffer = GetFrameBufferFromInput(1);
	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc();
	desc.InitOLD(false, false, false);
	desc.ShaderInUse = ShaderComplier::GetShader<Shader_DebugOutput>();
	desc.FrameBufferTarget = FB;
	DebugList->SetPipelineStateDesc(desc);
	DebugList->BeginRenderPass(RHIRenderPassDesc(FB, ERenderPassLoadOp::Clear));
	int VisAlpha = 0;

	if (currentDebugType == ERenderDebugOutput::GBuffer_RoughNess)
	{ 
		DebugList->SetFrameBufferTexture(gBuffer, 0, 2);
		VisAlpha = 1;
	}
	else if (currentDebugType == ERenderDebugOutput::GBuffer_Metallic)
	{
		DebugList->SetFrameBufferTexture(gBuffer, 0, 1);
		VisAlpha = 1;
	}
	else
	{
		DebugList->SetFrameBufferTexture(gBuffer, 0, currentDebugType - 1);
	}
	DebugList->SetRootConstant(1, 1, &VisAlpha, 0);
	RenderingUtils::RenderScreenQuad(DebugList);
	DebugList->EndRenderPass();
	gBuffer->MakeReadyForComputeUse(DebugList, true);
}

void VisModeNode::RenderForwardMode(ERenderDebugOutput::Type mode)
{

}

void VisModeNode::RenderScreenSpaceModes(ERenderDebugOutput::Type mode)
{

}
