#include "VisModeNode.h"
#include "Core/Assets/ShaderComplier.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/RenderingUtils.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/Shaders/PostProcess/Shader_DebugOutput.h"
#include "../../RayTracing/Voxel/VoxelTracingEngine.h"
#include "../../Shaders/Shader_Pair.h"
#include "../../RayTracing/Voxel/VoxelScene.h"
#include "../../Core/SceneRenderer.h"

VisModeNode::VisModeNode()
{
	SetNodeActive(false);
	OnNodeSettingChange();
	//RHI::GetRenderSettings()->SetDebugRenderMode(ERenderDebugOutput::Scene_Voxel);
}

VisModeNode::~VisModeNode()
{}

void VisModeNode::OnExecute()
{
	PassNodeThough(0);
	ERenderDebugOutput::Type mode = RHI::GetRenderSettings()->GetDebugRenderMode();
	DebugList = Context->GetListPool()->GetCMDList();
	SetBeginStates(DebugList);
	FrameBuffer* FB = GetFrameBufferFromInput(0);
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
	case ERenderDebugOutput::Scene_Voxel:
		if (SceneRenderer::Get()->GetVoxelScene())
		{
			SceneRenderer::Get()->GetVoxelScene()->RenderVoxelDebug(DebugList, FB);
		}
		//VoxelTracingEngine::Get()->RenderVoxelDebug(DebugList, FB,VoxelTracingEngine::Get()->VoxelMap);
		break;
	case ERenderDebugOutput::RelfectionBuffer:
		VisTexturesimple(mode);
		break;
	case ERenderDebugOutput::Scene_EdgeDetect:	
		VisTexturesimple(mode);
		break;
	case ERenderDebugOutput::Scene_EdgeDetectCount:
		break;
	}
	SetEndStates(DebugList);
}


void VisModeNode::RefreshNode()
{
	SetNodeActive(RHI::GetRenderSettings()->GetDebugRenderMode());
}

void VisModeNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::RenderTarget, StorageFormats::DontCare, "OutputBuffer");
	AddResourceInput(EStorageType::Framebuffer, EResourceState::PixelShader, StorageFormats::GBufferData, "GBuffer data");
	AddResourceInput(EStorageType::Framebuffer, EResourceState::PixelShader, StorageFormats::DontCare, "EdgeTex");


	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene);
	LinkThough(0);
}

void VisModeNode::OnSetupNode()
{
	DebugList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
void VisModeNode::VisTexturesimple(ERenderDebugOutput::Type mode)
{
	if (!RHI::GetRenderSettings()->GetVRXSettings().UseVRX() && mode != ERenderDebugOutput::RelfectionBuffer)
	{
		return;
	}
	if (!GetInput(1)->IsValid() || !GetInput(2)->IsValid())
	{
		return;
	}
	FrameBuffer* FB = GetFrameBufferFromInput(0);
	FrameBuffer* target = GetFrameBufferFromInput(2);
	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc();
	desc.InitOLD(false, false, false);
	desc.ShaderInUse = ShaderComplier::GetShader<Shader_DebugOutput>();
	desc.RenderTargetDesc = FB->GetPiplineRenderDesc();
	DebugList->SetPipelineStateDesc(desc);

	int VisAlpha = 0;

	if (mode == ERenderDebugOutput::Scene_EdgeDetect)
	{
		DebugList->SetFrameBufferTexture(target, 0);
	}
	if (mode == ERenderDebugOutput::Scene_EdgeDetectCount)
	{
		if (Pair == nullptr)
		{
			Pair = new Shader_Pair(RHI::GetDefaultDevice(), { "Deferred_LightingPass_vs","VRX/VRX_EdgeTileCount_Debug_PS" }, { EShaderType::SHADER_VERTEX,EShaderType::SHADER_FRAGMENT });
		}
		desc.ShaderInUse = Pair;
		DebugList->SetPipelineStateDesc(desc);
		DebugList->SetFrameBufferTexture(target, 0);
	}
	if (mode == ERenderDebugOutput::RelfectionBuffer)
	{
		DebugList->SetFrameBufferTexture(target, 0);
	}
	DebugList->BeginRenderPass(RHIRenderPassDesc(FB, ERenderPassLoadOp::Clear));
	if (mode != ERenderDebugOutput::Scene_EdgeDetectCount)
	{
		DebugList->SetRootConstant(1, 1, &VisAlpha, 0);
	}
	RenderingUtils::RenderScreenQuad(DebugList);
	DebugList->EndRenderPass();
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
	desc.RenderTargetDesc = FB->GetPiplineRenderDesc();
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
