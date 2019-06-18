#include "BasePassMeshProcessor.h"
#include "MeshBatch.h"
#include "MeshDrawCommand.h"
#include "Rendering/Core/Material.h"
#include "RHI/RHICommandList.h"


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

void BasePassMeshProcessor::OnSubmitCommands(RHICommandList* List, MeshDrawCommand* Command)
{
	if (Command->TargetMaterial != nullptr)
	{
		Command->TargetMaterial->SetMaterialActive(List, PassType);
	}
	else
	{
		Material::GetDefaultMaterial()->SetMaterialActive(List, PassType);
	}
}
