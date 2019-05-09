#include "ZPrePassMeshProcessor.h"
#include "Core\Assets\ShaderComplier.h"
#include "RHI\RHICommandList.h"
#include "MeshDrawCommand.h"
#include "..\SceneRenderer.h"
#include "MeshBatch.h"


ZPrePassMeshProcessor::ZPrePassMeshProcessor()
{
	Init();
}

ZPrePassMeshProcessor::~ZPrePassMeshProcessor()
{}

void ZPrePassMeshProcessor::Init()
{

}
void ZPrePassMeshProcessor::AddBatch(MeshBatch* Batch)
{
	if (Batch->MainPassCulled)
	{
		return;
	}
	Process(Batch);
}

void ZPrePassMeshProcessor::Process(MeshBatch* Batch)
{
	for (int i = 0; i < Batch->elements.size(); i++)
	{
		if (!Batch->elements[i]->IsVisible)
		{
			continue;
		}
		if (Batch->elements[i]->bTransparent)
		{
			continue;
		}
		MeshDrawCommand* command = new MeshDrawCommand();
		command->NumInstances = 1;
		command->NumPrimitves = Batch->elements[i]->NumPrimitives;
		command->Index = Batch->elements[i]->IndexBuffer;
		command->Vertex = Batch->elements[i]->VertexBuffer;
		command->TransformUniformBuffer = Batch->elements[i]->TransformBuffer;
		AddDrawCommand(command);
	}
}

void ZPrePassMeshProcessor::SubmitCommands(RHICommandList* List, Shader* shader)
{
	for (int i = 0; i < DrawCommands.size(); i++)
	{
		MeshDrawCommand* C = DrawCommands[i];
		List->SetConstantBufferView(C->TransformUniformBuffer, 0, 0);
		List->SetVertexBuffer(C->Vertex);
		List->SetIndexBuffer(C->Index);
		List->DrawIndexedPrimitive(C->NumPrimitves, C->NumInstances, 0, 0, 0);
		CountDrawCall();
	}

}

