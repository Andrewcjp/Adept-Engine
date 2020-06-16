#include "MeshPipelineController.h"
#include "BasePassMeshProcessor.h"
#include "Core/Assets/Scene.h"
#include "DepthOnlyMeshProcessor.h"
#include "MeshBatch.h"
#include "TransparentPassMeshProcessor.h"
#include "ZPrePassMeshProcessor.h"
#include "Core/Performance/PerfManager.h"
#include "MeshInstanceBuffer.h"
#include "RHI/RHITimeManager.h"
#include "../Material.h"
#include "VoxelPassMeshProcessor.h"


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
	explicit DistanceSort(glm::vec3 camerapos)
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
	for (int i = Batches.size() - 1; i >= 0; i--)
	{
		if (Batches[i]->Owner == nullptr)
		{
			Batches.erase(Batches.begin() + i);
		}
	}
	for (int i = 0; i < Batches.size(); i++)
	{
		if (Batches[i]->NeedRecreate())
		{
			MeshBatch* newBatch = Batches[i]->Owner->GetMesh()->GetMeshBatch();
			Batches[i]->IsValid = false;
			DeadBatches.push_back(newBatch);
			Batches[i] = newBatch;
		}
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
			MeshBatch* batch = CurrentObj->GetMesh()->GetMeshBatch();
			if (batch != nullptr && batch->Owner != nullptr && batch->elements.size() > 0) 
			{
				Batches.push_back(batch);
			}
			else
			{
				float t = 0;
			}
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
	MemoryUtils::DeleteVector(DeadBatches);
}
void MeshPipelineController::RemoveBatches(GameObject* owner)
{
	for (int i = Batches.size()-1; i >= 0; i--)
	{
		if (Batches[i]->Owner == owner)
		{
			Batches.erase(Batches.begin() + i);
		}
	}
}

void MeshPipelineController::ClearBatches()
{
	Batches.clear();
}

void MeshPipelineController::BuildStaticInstancing()
{
	if (!RHI::GetRenderSettings()->AllowMeshInstancing)
	{
		return;
	}
	//#mesh Group based on material
	//#mesh arbitrary per instance data.
	std::map<RHIBufferGroup*, std::vector<MeshBatch*>> Buckets;
	for (int i = 0; i < Batches.size(); i++)
	{
		if (Batches[i]->Owner == nullptr)
		{
			LogEnsure_Always("Batch orphaned");
			continue;
		}
		if (Batches[i]->Owner->GetMobility() != GameObject::Static)
		{
			continue;
		}
		if (Batches[i]->elements[0]->MaterialInUse == nullptr ||!Batches[i]->elements[0]->MaterialInUse->IsValidForInstancing())
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
			CreateInstanceController(Ctl, itor, (int)itor->second.size(), 0);
		}
	}
}

void MeshPipelineController::CreateInstanceController(MeshBatch* Ctl, std::map<RHIBufferGroup *, std::vector<MeshBatch *>>::iterator itor, int limit, int offset)
{
	Ctl->IsinstancedBatch = true;
	Ctl->InstanceBuffer = new MeshInstanceBuffer();
	Ctl->InstanceBuffer->TargetMaterial = Ctl->elements[0]->MaterialInUse;
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
	MeshPassRenderArgs args;
	args.PassType = type;
	args.UseDeferredShaders = false;
	args.UseShadows = false;
	RenderPass(args, List, shader, Filter);
}

void MeshPipelineController::RenderPass(const MeshPassRenderArgs & args, RHICommandList* List, Shader* shader /*= nullptr*/, EBatchFilter::Type Filter /*= EBatchFilter::ALL*/)
{
	ERenderPass::Type type = args.PassType;
	SCOPE_CYCLE_COUNTER_GROUP(ERenderPass::ToString(args.PassType).c_str(), "Render");
	//DECALRE_SCOPEDGPUCOUNTER(List, ERenderPass::ToString(args.PassType));
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
		Processors[type]->SubmitCommands(List, args);
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
	Processors[ERenderPass::VoxelPass] = new VoxelPassMeshProcessor();
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
		case ERenderPass::VoxelPass:
			return "VoxelPass";
	}
	return std::string();
}
