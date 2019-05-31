#include "Stdafx.h"
#include "BasePassMeshProcessor.h"
#include "RHI\RHICommandList.h"
#include "MeshDrawCommand.h"
#include "MeshBatch.h"
#include "..\Material.h"
#include "..\..\MeshInstanceBuffer.h"


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
	Process(Batch);
}

//void BasePassMeshProcessor::Process(MeshBatch* Batch)
//{
//	for (int i = 0; i < Batch->elements.size(); i++)
//	{
//		if (!Batch->elements[i]->IsVisible)
//		{
//			continue;
//		}
//		//transparent elements are rendered after opaque.
//		if (Batch->elements[i]->bTransparent)
//		{
//			continue;
//		}
//
//		MeshDrawCommand* command = new MeshDrawCommand();
//		if (Batch->InstanceBuffer != nullptr)
//		{
//			command->NumInstances = Batch->InstanceBuffer->GetInstanceCount();
//		}
//		else
//		{
//			command->NumInstances = 1;
//		}
//		command->NumPrimitves = Batch->elements[i]->NumPrimitives;
//		command->Index = Batch->elements[i]->IndexBuffer;
//		command->Vertex = Batch->elements[i]->VertexBuffer;
//		if (Batch->IsinstancedBatch)
//		{
//			command->TransformUniformBuffer = Batch->InstanceBuffer->GetBuffer();
//		}
//		else
//		{
//			command->TransformUniformBuffer = Batch->elements[i]->TransformBuffer;
//		}
//		command->TargetMaterial = Batch->elements[i]->MaterialInUse;
//		AddDrawCommand(command);
//	}
//}
//
//void BasePassMeshProcessor::SubmitCommands(RHICommandList* List, Shader* shader)
//{
//	for (int i = 0; i < DrawCommands.size(); i++)
//	{
//		MeshDrawCommand* C = DrawCommands[i];
//		if (C->TargetMaterial != nullptr)
//		{
//			C->TargetMaterial->SetMaterialActive(List, PassType);
//		}
//		else
//		{
//			Material::GetDefaultMaterial()->SetMaterialActive(List, PassType);
//		}
//		List->SetConstantBufferView(C->TransformUniformBuffer, 0, 0);
//		List->SetVertexBuffer(C->Vertex);
//		List->SetIndexBuffer(C->Index);
//		List->DrawIndexedPrimitive(C->NumPrimitves, C->NumInstances, 0, 0, 0);
//		CountDrawCall();
//	}
//}

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
