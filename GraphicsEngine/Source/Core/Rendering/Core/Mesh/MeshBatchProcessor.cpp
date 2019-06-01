#include "MeshBatchProcessor.h"
#include "Core/Performance/PerfManager.h"
#include "MeshBatch.h"
#include "MeshDrawCommand.h"

static ConsoleVariable UseDynamicInstancing("r.UseDynamicInstancing", 0, ECVarType::ConsoleOnly);

MeshBatchProcessor::MeshBatchProcessor()
{
	PerfManager::Get()->AddTimer("DrawCalls", "Render");
}

MeshBatchProcessor::~MeshBatchProcessor()
{
	Reset();
}

void MeshBatchProcessor::AddBatch(MeshBatch * Batch)
{
	//check batch is relevant!
	//then process 
}

//void MeshBatchProcessor::Process(MeshBatch* batch)
//{
//	//build a draw command for this batch
//}

void MeshBatchProcessor::Reset()
{
	MemoryUtils::DeleteVector(DrawCommands);
	DrawCommands.clear();
	DrawCallsThisFrame = 0;
}

void MeshBatchProcessor::UpdateStats()
{
	PerfManager::AddToCountTimer("DrawCalls", DrawCallsThisFrame);
}

bool MeshBatchProcessor::CheckProcess(MeshBatchElement * Element)
{
	return true;
}

bool MeshBatchProcessor::IsSubInstance(MeshBatch * Batch)
{
	if (DisableInstancing)
	{
		return false;
	}
	if (Batch->IsinstancedBatch && Batch->InstanceOwner != nullptr)
	{
		return true;
	}
	return false;
}

void MeshBatchProcessor::AddDrawCommand(MeshDrawCommand * cmd)
{
	DrawCommands.push_back(cmd);
}

//void MeshBatchProcessor::MergeCommands()
//{
//	if (!UseDynamicInstancing.GetBoolValue())
//	{
//		return;
//	}
//	//check all commands and merge were possible
//	//same mesh same material
//	//equal same draw call
//}

void MeshBatchProcessor::CountDrawCall()
{
	DrawCallsThisFrame++;
}

void MeshBatchProcessor::Process(MeshBatch* Batch)
{
	for (int i = 0; i < Batch->elements.size(); i++)
	{
		if (!Batch->elements[i]->IsVisible)
		{
			continue;
		}
		//transparent elements are rendered after opaque.
		if (!CheckProcess(Batch->elements[i]))
		{
			continue;
		}
		MeshDrawCommand* command = new MeshDrawCommand();
		if (Batch->InstanceBuffer != nullptr && !DisableInstancing)
		{
			command->NumInstances = Batch->InstanceBuffer->GetInstanceCount();
		}
		else
		{
			command->NumInstances = 1;
		}
		command->NumPrimitves = Batch->elements[i]->NumPrimitives;
		command->Index = Batch->elements[i]->IndexBuffer;
		command->Vertex = Batch->elements[i]->VertexBuffer;
		if (Batch->InstanceBuffer != nullptr && !DisableInstancing)
		{
			command->TransformUniformBuffer = Batch->InstanceBuffer->GetBuffer();
		}
		else
		{
			command->TransformUniformBuffer = Batch->elements[i]->TransformBuffer;
		}
		command->TargetMaterial = Batch->elements[i]->MaterialInUse;
		AddDrawCommand(command);
	}
}

void MeshBatchProcessor::SubmitCommands(RHICommandList* List, Shader* shader)
{
	for (int i = 0; i < DrawCommands.size(); i++)
	{
		MeshDrawCommand* C = DrawCommands[i];
		OnSubmitCommands(List, C);
		List->SetConstantBufferView(C->TransformUniformBuffer, 0, 0);
		List->SetVertexBuffer(C->Vertex);
		List->SetIndexBuffer(C->Index);
		List->DrawIndexedPrimitive(C->NumPrimitves, C->NumInstances, 0, 0, 0);
		CountDrawCall();
	}
}

void MeshBatchProcessor::OnSubmitCommands(RHICommandList * List, MeshDrawCommand * Command)
{

}
