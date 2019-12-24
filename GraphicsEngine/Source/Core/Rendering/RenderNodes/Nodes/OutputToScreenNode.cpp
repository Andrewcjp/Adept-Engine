#include "OutputToScreenNode.h"
#include "Core/Assets/ShaderComplier.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Rendering/Core/RenderingUtils.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/Shaders/PostProcess/Shader_Compost.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHICommandList.h"
#include "../NodeLink.h"
#include "../StoreNodes/FrameBufferStorageNode.h"
#include "../../Core/FrameBuffer.h"
#include "UI/UIManager.h"
#include "RHI/RHITexture.h"
static ConsoleVariable VROutputMode("vr.screenmode", 0, ECVarType::ConsoleAndLaunch);
OutputToScreenNode::OutputToScreenNode()
{
	OnNodeSettingChange();
}


OutputToScreenNode::~OutputToScreenNode()
{
	SafeRHIRelease(ScreenWriteList);
}

void OutputToScreenNode::OnExecute()
{
	FrameBuffer* Target = GetFrameBufferFromInput(0);
	FrameBufferStorageNode* FBNode = ((FrameBufferStorageNode*)GetInput(0)->GetStoreTarget());
	const bool IsVRFb = FBNode->IsVRFramebuffer;

	ScreenWriteList = Context->GetListPool()->GetCMDList();
	SetBeginStates(ScreenWriteList);
#if WITH_EDITOR
	RHIRenderPassDesc RP = RHIRenderPassDesc(GetFrameBufferFromInput(1), ERenderPassLoadOp::Clear);
	RP.InitalState = GPU_RESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET;
	GetFrameBufferFromInput(1)->SetResourceState(ScreenWriteList, EResourceState::RenderTarget);
#else
	RHIRenderPassDesc RP = RHI::GetRenderPassDescForSwapChain(true);
	RP.InitalState = GPU_RESOURCE_STATES::RESOURCE_STATE_UNDEFINED;
#endif

	ScreenWriteList->BeginRenderPass(RP);
	if (IsVRFb)
	{
		if (VROutputMode.GetIntValue() == 0)
		{
			RHIPipeLineStateDesc D = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_VROutput>());
			D.Cull = false;
			D.DepthStencilState.DepthEnable = false;
			D.RenderTargetDesc = RHIPipeRenderTargetDesc::GetDefault();
			ScreenWriteList->SetPipelineStateDesc(D);
			FBNode->GetFramebuffer(EEye::Left)->SetResourceState(ScreenWriteList, EResourceState::PixelShader);
			FBNode->GetFramebuffer(EEye::Right)->SetResourceState(ScreenWriteList, EResourceState::PixelShader);
			ScreenWriteList->SetFrameBufferTexture(FBNode->GetFramebuffer(EEye::Left), 0);
			ScreenWriteList->SetFrameBufferTexture(FBNode->GetFramebuffer(EEye::Right), 1);
		}
		else
		{
			RHIPipeLineStateDesc D = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_Compost>());
			D.Cull = false;
			D.DepthStencilState.DepthEnable = false;
			D.RenderTargetDesc = RHIPipeRenderTargetDesc::GetDefault();
			ScreenWriteList->SetPipelineStateDesc(D);
			if (VROutputMode.GetIntValue() == 1)
			{
				ScreenWriteList->SetFrameBufferTexture(FBNode->GetFramebuffer(EEye::Left), 0);
			}
			else if (VROutputMode.GetIntValue() == 2)
			{
				ScreenWriteList->SetFrameBufferTexture(FBNode->GetFramebuffer(EEye::Right), 0);
			}
		}
	}
	else
	{
		RHIPipeLineStateDesc D = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_Compost>());
		D.Cull = false;
		D.DepthStencilState.DepthEnable = false;
		D.RenderTargetDesc = RHIPipeRenderTargetDesc::GetDefault();
		ScreenWriteList->SetPipelineStateDesc(D);
		ScreenWriteList->SetFrameBufferTexture(Target, 0);
	}
	RenderingUtils::RenderScreenQuad(ScreenWriteList);
	ScreenWriteList->EndRenderPass();
#if WITH_EDITOR
	GetFrameBufferFromInput(1)->SetResourceState(ScreenWriteList, EResourceState::PixelShader);
	UIManager::Get()->SetEditorViewPortRenderTarget(GetFrameBufferFromInput(1));
#endif
	SetEndStates(ScreenWriteList);
	Context->GetListPool()->Flush();
}

void OutputToScreenNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::PixelShader, StorageFormats::DontCare, "Frame");
#if WITH_EDITOR
	AddResourceInput(EStorageType::Framebuffer, EResourceState::PixelShader, StorageFormats::DontCare, "Editor ViewPort Output");
#endif
}

void OutputToScreenNode::OnSetupNode()
{
	ScreenWriteList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
