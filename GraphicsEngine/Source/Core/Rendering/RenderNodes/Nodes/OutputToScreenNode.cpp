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
	ScreenWriteList->ResetList();
	Target->SetResourceState(ScreenWriteList, EResourceState::PixelShader);
	RHIRenderPassDesc RP = RHI::GetRenderPassDescForSwapChain(true);
	RP.InitalState = GPU_RESOURCE_STATES::RESOURCE_STATE_UNDEFINED;
	ScreenWriteList->BeginRenderPass(RP);
	if (IsVRFb)
	{
		if (VROutputMode.GetIntValue() == 0)
		{
			RHIPipeLineStateDesc D = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_VROutput>());
			D.Cull = false;
			ScreenWriteList->SetPipelineStateDesc(D);
			ScreenWriteList->SetFrameBufferTexture(FBNode->GetFramebuffer(EEye::Left), 0);
			ScreenWriteList->SetFrameBufferTexture(FBNode->GetFramebuffer(EEye::Right), 1);
		}
		else
		{
			RHIPipeLineStateDesc D = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_Compost>());
			D.Cull = false;
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
		ScreenWriteList->SetPipelineStateDesc(D);
		ScreenWriteList->SetFrameBufferTexture(Target, 0);
	}
	RenderingUtils::RenderScreenQuad(ScreenWriteList);
	ScreenWriteList->EndRenderPass();
	ScreenWriteList->Execute();
}

void OutputToScreenNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::DontCare, "Frame");
}

void OutputToScreenNode::OnSetupNode()
{
	ScreenWriteList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
