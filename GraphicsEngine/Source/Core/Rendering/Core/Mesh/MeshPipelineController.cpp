#include "Stdafx.h"
#include "MeshPipelineController.h"
#include "Core\Assets\Scene.h"
#include "DepthOnlyMeshProcessor.h"
#include "Core\Performance\PerfManager.h"
#include "BasePassMeshProcessor.h"
#include "MeshBatch.h"
#include "TransparentPassMeshProcessor.h"
#include "ZPrePassMeshProcessor.h"

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
struct DistanceSort
{
	glm::vec3 pos;
	DistanceSort(glm::vec3 camerapos)
	{
		pos = camerapos;
	}
	bool operator()(MeshBatch* a, MeshBatch*b) const
	{
		float distance = glm::distance2(a->Owner->GetBounds()->ClosestPoint(pos), pos);
		float distance2 = glm::distance2(b->Owner->GetBounds()->ClosestPoint(pos), pos);
		return distance < distance2;
	}
};

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
	//#mesh custom sort params
	//sort by closest point in shape
#if !_DEBUG
	{
		SCOPE_CYCLE_COUNTER_GROUP("Sort Batches", "Render");
		std::sort(Batches.begin(), Batches.end(), DistanceSort(TargetScene->GetCurrentRenderCamera()->GetPosition()));
	}
#endif
}

void MeshPipelineController::RenderPass(ERenderPass::Type type, RHICommandList* List, Shader* shader, EBatchFilter::Type Filter)
{
	SCOPE_CYCLE_COUNTER_GROUP((type == ERenderPass::DepthOnly) ? "Depth RenderPass" : "Base RenderPass", "Render");
	Processors[type]->Reset();
	for (int i = 0; i < Batches.size(); i++)
	{
		if (Filter == EBatchFilter::StaticOnly)
		{
			if (Batches[i]->Owner->GetMobility() != GameObject::Static)
			{
				continue;
			}
		}
		else if (Filter == EBatchFilter::DynamicOnly)
		{
			if (Batches[i]->Owner->GetMobility() != GameObject::Dynamic)
			{
				continue;
			}
		}
		Processors[type]->AddBatch(Batches[i]);
	}
	Processors[type]->SubmitCommands(List, shader);
	Processors[type]->UpdateStats();
}

void MeshPipelineController::Init()
{
	Processors[ERenderPass::DepthOnly] = new DepthOnlyMeshProcessor();
	Processors[ERenderPass::TransparentPass] = new TransparentPassMeshProcessor();
	Processors[ERenderPass::PreZ] = new ZPrePassMeshProcessor();
	Processors[ERenderPass::BasePass] = new BasePassMeshProcessor();
	Processors[ERenderPass::BasePass_Cubemap] = new BasePassMeshProcessor(true);
}
