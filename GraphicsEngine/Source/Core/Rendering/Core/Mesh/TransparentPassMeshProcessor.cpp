#include "Stdafx.h"
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
	Process(Batch);
}

void TransparentPassMeshProcessor::Process(MeshBatch* Batch)
{
	for (int i = 0; i < Batch->elements.size(); i++)
	{
		if (!Batch->elements[i]->IsVisible)
		{
			continue;
		}
		//transparent elements are rendered after opaque.
		if (!Batch->elements[i]->bTransparent)
		{
			continue;
		}
		MeshDrawCommand* command = new MeshDrawCommand();
		command->NumInstances = 1;
		command->NumPrimitves = Batch->elements[i]->NumPrimitives;
		command->Index = Batch->elements[i]->IndexBuffer;
		command->Vertex = Batch->elements[i]->VertexBuffer;
		command->TransformUniformBuffer = Batch->elements[i]->TransformBuffer;
		command->TargetMaterial = Batch->elements[i]->MaterialInUse;
		AddDrawCommand(command);
	}
}

void TransparentPassMeshProcessor::SubmitCommands(RHICommandList* List, Shader* shader)
{
	for (int i = 0; i < DrawCommands.size(); i++)
	{
		MeshDrawCommand* C = DrawCommands[i];
		if (C->TargetMaterial != nullptr)
		{
			C->TargetMaterial->SetMaterialActive(List, PassType);
		}
		else
		{
			Material::GetDefaultMaterial()->SetMaterialActive(List, PassType);
		}
		List->SetConstantBufferView(C->TransformUniformBuffer, 0, 0);
		List->SetVertexBuffer(C->Vertex);
		List->SetIndexBuffer(C->Index);
		List->DrawIndexedPrimitive(C->NumPrimitves, C->NumInstances, 0, 0, 0);
		CountDrawCall();
	}
}
