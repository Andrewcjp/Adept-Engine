#include "TransparentPassMeshProcessor.h"
#include "RHI\RHICommandList.h"
#include "MeshDrawCommand.h"
#include "MeshBatch.h"
#include "..\Material.h"


TransparentPassMeshProcessor::TransparentPassMeshProcessor()
{
	PassType = ERenderPass::TransparentPass;
}


TransparentPassMeshProcessor::~TransparentPassMeshProcessor()
{}

void TransparentPassMeshProcessor::AddBatch(MeshBatch* Batch)
{
	if (IsSubInstance(Batch))
	{
		return;
	}
	Process(Batch);
}

void TransparentPassMeshProcessor::OnSubmitCommands(RHICommandList * List, MeshDrawCommand * Command)
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

bool TransparentPassMeshProcessor::CheckProcess(MeshBatchElement* Element)
{
	if (!Element->bTransparent)
	{
		return false;
	}
	return true;
}
