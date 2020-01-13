#include "BasePassMeshProcessor.h"
#include "MeshBatch.h"
#include "MeshDrawCommand.h"
#include "Rendering/Core/Material.h"
#include "RHI/RHICommandList.h"
#include "../../Shaders/Shader_Main.h"
#include "../SceneRenderer.h"


BasePassMeshProcessor::BasePassMeshProcessor(bool Cubemap)
{
	IsForCubemap = Cubemap;
	if (IsForCubemap)
	{
		PassType = ERenderPass::BasePass_Cubemap;
	}
	else
	{
		PassType = ERenderPass::BasePass;
	}
}

BasePassMeshProcessor::~BasePassMeshProcessor()
{}

void BasePassMeshProcessor::AddBatch(MeshBatch* Batch)
{
	if (Batch->MainPassCulled && !IsForCubemap && !Batch->IsinstancedBatch)
	{
		return;
	}
	if (IsSubInstance(Batch))
	{
		return;
	}
	if (Batch->elements[0]->bTransparent)
	{
		return;
	}
	Process(Batch);
}

void BasePassMeshProcessor::OnSubmitCommands(RHICommandList* List, MeshDrawCommand* Command, const MeshPassRenderArgs& args)
{
	if (Command->TargetMaterial != nullptr)
	{
		Command->TargetMaterial->SetMaterialActive(List, args);
	}
	else
	{
		Material::GetDefaultMaterial()->SetMaterialActive(List, args);
	}
	if (Command->NumInstances > 1)
	{
		//todo: move this 
		List->SetConstantBufferView(Command->MaterialInstanceBuffer, 0, "MateralConstantBuffer");
	}

	SceneRenderer::Get()->BindMvBuffer(List, "", args.Eye);
}
