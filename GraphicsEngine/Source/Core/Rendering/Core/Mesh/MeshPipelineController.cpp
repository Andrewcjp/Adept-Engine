#include "MeshPipelineController.h"
#include "BasePassMeshProcessor.h"
#include "Core/Assets/Scene.h"
#include "DepthOnlyMeshProcessor.h"
#include "MeshBatch.h"
#include "TransparentPassMeshProcessor.h"
#include "ZPrePassMeshProcessor.h"
#include "Core/Performance/PerfManager.h"
#include "MeshInstanceBuffer.h"


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
	for (int i = 0; i < Batches.size(); i++)
	{
		Batches[i]->Update();
	}
	if (TargetScene->ObjectsAddedLastFrame.size() == 0)
	{
		return;
	}
	//#MESH: handle removal of scene objects
	for (int i = 0; i < TargetScene->ObjectsAddedLastFrame.size(); i++)
	{
		GameObject* CurrentObj = TargetScene->ObjectsAddedLastFrame[i];
		if (CurrentObj->GetMesh() != nullptr)
		{
			Batches.push_back(CurrentObj->GetMesh()->GetMeshBatch());
		}
	}
	TargetScene->ObjectsAddedLastFrame.clear();
	//#mesh custom sort params
	//sort by closest point in shape
#if !_DEBUG
	{
		SCOPE_CYCLE_COUNTER_GROUP("Sort Batches", "Render");
		std::sort(Batches.begin(), Batches.end(), DistanceSort(TargetScene->GetCurrentRenderCamera()->GetPosition()));
	}
#endif

	if (RHI::GetFrameCount() == 0)
	{
		BuildStaticInstancing();
	}
}

void MeshPipelineController::BuildStaticInstancing()
{
	if (!RHI::GetRenderSettings()->AllowMeshInstancing)
	{
		return;
	}
	//#mesh Group based on material
	//#mesh arbitrary per instance data.
	std::map<RHIBuffer*, std::vector<MeshBatch*>> Buckets;
	for (int i = 0; i < Batches.size(); i++)
	{
		if (Batches[i]->Owner->GetMobility() != GameObject::Static)
		{
			continue;
		}

		auto itor = Buckets.find(Batches[i]->elements[0]->VertexBuffer);
		if (itor != Buckets.end())
		{
			itor->second.push_back(Batches[i]);
		}
		else
		{
			Buckets.emplace(Batches[i]->elements[0]->VertexBuffer, std::vector<MeshBatch*>{ Batches[i] });
		}
	}

	for (auto itor = Buckets.begin(); itor != Buckets.end(); itor++)
	{
		if (itor->second.size() < 2)
		{
			continue;
		}
		MeshBatch* Ctl = itor->second[0];
		if (itor->second.size() >= RHI::GetRenderConstants()->MAX_MESH_INSTANCES)
		{
			const int maxinstance = RHI::GetRenderConstants()->MAX_MESH_INSTANCES;
			for (int i = 0; i < itor->second.size(); i += maxinstance)
			{
				CreateInstanceController(itor->second[i], itor, maxinstance, i);
			}
		}
		else
		{
			CreateInstanceController(Ctl, itor, itor->second.size(), 0);
		}
	}
}

void MeshPipelineController::CreateInstanceController(MeshBatch* Ctl, std::map<RHIBuffer *, std::vector<MeshBatch *>>::iterator itor, int limit, int offset)
{
	Ctl->IsinstancedBatch = true;
	Ctl->InstanceBuffer = new MeshInstanceBuffer();
	Ctl->InstanceBuffer->AddBatch(Ctl);
	for (int i = 1 + offset; i < glm::min((int)itor->second.size(), offset + limit); i++)
	{
		Ctl->InstanceBuffer->AddBatch(itor->second[i]);
		itor->second[i]->InstanceOwner = Ctl;
		itor->second[i]->IsinstancedBatch = true;
	}
	Ctl->InstanceBuffer->Build();
	Ctl->InstanceBuffer->UpdateBuffer();
}

void MeshPipelineController::RenderPass(ERenderPass::Type type, RHICommandList* List, Shader* shader, EBatchFilter::Type Filter)
{
	SCOPE_CYCLE_COUNTER_GROUP(ERenderPass::ToString(type).c_str(), "Render");
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
	{
		SCOPE_CYCLE_COUNTER_GROUP("SubmitCommands CPU", "Render");
		Processors[type]->SubmitCommands(List, shader);
	}
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

std::string ERenderPass::ToString(ERenderPass::Type t)
{
	switch (t)
	{
		case ERenderPass::DepthOnly:
			return "DepthOnly";
		case ERenderPass::BasePass:
			return "BasePass";;
		case ERenderPass::BasePass_Cubemap:
			return "BasePass_Cubemap";;
		case ERenderPass::TransparentPass:
			return "TransparentPass";;
		case ERenderPass::PreZ:
			return "PreZ";
	}
	return std::string();
}
