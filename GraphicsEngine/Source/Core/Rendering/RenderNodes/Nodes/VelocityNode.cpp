#include "VelocityNode.h"
#include "../StorageNodeFormats.h"
#include "../../Core/SceneRenderer.h"
#include "RHI/RHICommandList.h"
#include "Core/Assets/ShaderComplier.h"
#include "Core/Performance/PerfManager.h"
#include "../NodeLink.h"
#include "../../Shaders/Shader_Pair.h"
#include "RHI/RHITimeManager.h"
#include "../../Shaders/Shader_Main.h"
#include "RHI/RHIBufferGroup.h"

VelocityNode::VelocityNode()
{
	OnNodeSettingChange();
}

VelocityNode::~VelocityNode()
{
}

void VelocityNode::OnExecute()
{
	SCOPE_CYCLE_COUNTER_GROUP("Velocity pass", "Render");
	FrameBuffer* Target = GetFrameBufferFromInput(0);
	Cmdlist->ResetList();
	SetBeginStates(Cmdlist);

	{
		DECALRE_SCOPEDGPUCOUNTER(Cmdlist,"Velocity pass");
		RHIPipeLineStateDesc desc; 
		desc = RHIPipeLineStateDesc::CreateDefault(FixedVelocityShaders, Target);
		desc.DepthStencilState.DepthCompareFunction = COMPARISON_FUNC_LESS; 
		desc.DepthStencilState.DepthWrite = true;
		desc.RenderPassDesc = RHIRenderPassDesc(Target, ERenderPassLoadOp::Clear);
		desc.InputLayout.Elements = Shader_Main::GetMainVertexFormat();
		Cmdlist->SetPipelineStateDesc(desc);
		Cmdlist->SetConstantBufferView(SceneRenderer::Get()->GetLastMVData()->Get(Cmdlist), 0, "LastFrameData");
		Cmdlist->BeginRenderPass(RHIRenderPassDesc(Target, ERenderPassLoadOp::Clear));

		SceneRenderer::Get()->RenderScene(Cmdlist, true, Target, false, GetEye());
		Cmdlist->EndRenderPass();
		
	}
	SetEndStates(Cmdlist);
	Cmdlist->Execute();

}

void VelocityNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::RenderTarget, StorageFormats::DefaultFormat,"Velocity Buffer");
}

void VelocityNode::OnSetupNode()
{
	Cmdlist = RHI::CreateCommandList(ECommandListType::Graphics, Context);
	FixedVelocityShaders = new Shader_Pair(Context, { "Velocity_vs", "VelocityWrite" }, { EShaderType::SHADER_VERTEX,EShaderType::SHADER_FRAGMENT });
}

