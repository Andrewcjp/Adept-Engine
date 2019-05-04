#include "Stdafx.h"
#include "MeshPipelineController.h"
#include "Core\Assets\Scene.h"
#include "DepthOnlyMeshProcessor.h"
#include "Core\Performance\PerfManager.h"
#include "BasePassMeshProcessor.h"


MeshPipelineController::MeshPipelineController()
{
	Init();
}

MeshPipelineController::~MeshPipelineController()
{
	for (int i = 0; i < ERenderPass::Limit; i++)
	{
		SafeDelete(Processors[i]);
	}
}

void MeshPipelineController::GatherBatches()
{
	if (TargetScene == nullptr)
	{
		return;
	}
	MemoryUtils::DeleteVector(Batches);
	Batches.clear();
	for (int i = 0; i < (*TargetScene->GetMeshObjects()).size(); i++)
	{
		GameObject* CurrentObj = (*TargetScene->GetMeshObjects())[i];
		if (CurrentObj->GetMesh() != nullptr)
		{
			Batches.push_back(CurrentObj->GetMesh()->GetMeshBatch());
		}
	}
}

void MeshPipelineController::RenderPass(ERenderPass::Type type, RHICommandList* List, Shader* shader)
{
	SCOPE_CYCLE_COUNTER_GROUP((type == ERenderPass::DepthOnly) ? "Depth RenderPass" : "Base RenderPass", "Render");
	Processors[type]->Reset();
	for (int i = 0; i < Batches.size(); i++)
	{
		Processors[type]->AddBatch(Batches[i]);
	}
	Processors[type]->SubmitCommands(List, shader);
	Processors[type]->UpdateStats();
}

void MeshPipelineController::Init()
{
	Processors[ERenderPass::DepthOnly] = new DepthOnlyMeshProcessor();
	Processors[ERenderPass::BasePass] = new BasePassMeshProcessor();
}
